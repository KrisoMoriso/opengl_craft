#include "World.h"
#include <glad/glad.h>
#include "Chunk.h"
#include "Game.h"
#define FASTNOISELITE_IMPLEMENTATION
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
#endif

#include "FastNoiseLite.h"

// Pop the state back to normal so your own code is still checked
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
#include <cmath>
#include <glm/glm.hpp>
#include <algorithm>
World::World(ThreadPool& threadPool, Renderer& renderer, int seed, int renderDistance, BlockRegistry &blockRegistryInstance)
: m_thread_pool(threadPool), m_renderer(renderer), m_blockRegistry(blockRegistryInstance)
{
    m_seed = seed;
    m_render_distance = renderDistance;
}

World::ChunkPos World::get_chunk_position(glm::vec3 position){
    ChunkPos chunk_pos;
    chunk_pos.x = static_cast<int>(std::floor(position.x / 16.0f));
    chunk_pos.y = static_cast<int>(std::floor(position.y / 16.0f));
    chunk_pos.z = static_cast<int>(std::floor(position.z / 16.0f));
    return chunk_pos;
}

unsigned short World::get_block_material(int x, int y, int z){
    int chunk_x = x >> 4;
    int chunk_y = y >> 4;
    int chunk_z = z >> 4;

    int block_x = x & 15;
    int block_y = y & 15;
    int block_z = z & 15;

    auto chunk_it = m_chunks.find({chunk_x, chunk_y, chunk_z});

    if (chunk_it != m_chunks.end()){
        return chunk_it->second->getBlockMaterial(block_x, block_y, block_z);
    }
    return 0;
}

void World::set_block_material(int x, int y, int z, unsigned short material){
    int chunk_x = x >> 4;
    int chunk_y = y >> 4;
    int chunk_z = z >> 4;

    int block_x = x & 15;
    int block_y = y & 15;
    int block_z = z & 15;

    auto chunk_it = m_chunks.find({chunk_x, chunk_y, chunk_z});

    if (chunk_it != m_chunks.end()){
        {
            std::unique_lock<std::shared_mutex> lock(chunk_it->second->m_block_mutex);
            chunk_it->second->setBlock(block_x, block_y, block_z, material);
        }
    }

}

void World::generate_world(glm::vec3 player_position){
     ChunkPos chunk_pos = get_chunk_position(player_position);

    if (chunk_pos != m_last_player_chunk) {

        int render_dist = m_render_distance + 4;
        for (const auto& pair : m_chunks) {
            ChunkPos pos = pair.first;
            if (std::abs(pos.x - chunk_pos.x) > render_dist or std::abs(pos.z - chunk_pos.z) > render_dist){
                m_queue_to_delete.push_back(pos);
            }
        }
        render_dist = m_render_distance;
        bool needs_sorting = false;
        for (int x = chunk_pos.x - render_dist; x <= chunk_pos.x + render_dist; ++x) {
            for (int y = 0; y < WORLD_CHUNK_HEIGHT; ++y) {
                for (int z = chunk_pos.z - render_dist; z <= chunk_pos.z + render_dist; ++z) {

                    if (!m_chunks.contains({x,y,z}) and !m_chunks_in_progress.contains({x, y, z})){
                        m_queue_to_generate.push_back({x, y, z});
                        m_chunks_in_progress.insert({x,y,z});
                        needs_sorting = true;
                    }
                }
            }
        }
        if (needs_sorting or true){
            std::sort(m_queue_to_generate.begin(), m_queue_to_generate.end(),
                [&chunk_pos](const ChunkPos& a, const ChunkPos& b) {
                    int distA = (a.x - chunk_pos.x)*(a.x - chunk_pos.x) +
                                (a.z - chunk_pos.z)*(a.z - chunk_pos.z)+
                                (a.z - chunk_pos.z)*(a.z - chunk_pos.z);
                    int distB = (b.x - chunk_pos.x)*(b.x - chunk_pos.x) +
                                (b.z - chunk_pos.z)*(b.z - chunk_pos.z)+
                                (b.z - chunk_pos.z)*(b.z - chunk_pos.z);
                    return distA > distB;
            });
        }
        int deleted = 0;
        for (const auto& pos : m_queue_to_delete) {
            // if (deleted >= 4) break;

            if (m_chunks.contains(pos) and !m_chunks[pos]->m_is_generating) {
                m_chunks.erase(pos);
                m_renderer.add_chunk_to_unload(pos);
                deleted++;
            }
        }
        m_queue_to_delete.clear();

        m_last_player_chunk = chunk_pos;
    }



    int constexpr max_generates = 4;
    int generates_this_frame = 0;
    while (!m_queue_to_generate.empty() and generates_this_frame < max_generates) {
        ChunkPos pos_queue = m_queue_to_generate.back();
        m_queue_to_generate.pop_back();

        GenerationJob job = {pos_queue};
        m_thread_pool.enqueue([this, job]() {
            this->generate_chunk(job, m_queue_generation_result);
        });

        generates_this_frame++;
    }


     GenerationResult finished_result;
    int constexpr max_finishes = 4;
    int finishes = 0;

    while (finishes < max_finishes and m_queue_generation_result.try_pop(finished_result) ) {
        m_chunks[finished_result.chunk_pos] = std::move(finished_result.chunk);
        m_chunks[finished_result.chunk_pos]->m_is_generating = false;
        m_chunks_in_progress.erase(finished_result.chunk_pos);
        finishes++;
    }
}

void World::generate_chunk(GenerationJob generation_job,
    ThreadPool::SafeQueue<GenerationResult>& queue_generation_result){

    FastNoiseLite noise_macro; //continentalism
    noise_macro.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise_macro.SetSeed(m_seed);
    noise_macro.SetFrequency(0.0013f);
    noise_macro.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise_macro.SetFractalOctaves(3);


    FastNoiseLite noise_micro;
    noise_micro.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise_micro.SetSeed(m_seed + 1);
    noise_micro.SetFrequency(0.005f);// smaller frequency for smaller bumps

    FastNoiseLite noise_caves;
    noise_caves.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise_caves.SetSeed(m_seed + 2);
    noise_caves.SetFrequency(0.015f); // frequency determines how tight the tunnels twist
    noise_caves.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise_caves.SetFractalOctaves(2);

    FastNoiseLite noise_cave_mask;
    noise_cave_mask.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise_cave_mask.SetSeed(m_seed + 3);
    noise_cave_mask.SetFrequency(0.005f);

    FastNoiseLite temperature;
    temperature.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    temperature.SetSeed(m_seed + 4);
    temperature.SetFrequency(0.001f);
    temperature.SetFractalType(FastNoiseLite::FractalType_FBm);
    temperature.SetFractalOctaves(4);

    FastNoiseLite humidity;
    humidity.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    humidity.SetSeed(m_seed + 10);
    humidity.SetFrequency(0.0015f);
    humidity.SetFractalType(FastNoiseLite::FractalType_FBm);
    humidity.SetFractalOctaves(4);

    GenerationResult result;
    result.chunk_pos = generation_job.chunk_pos;
    result.chunk = std::make_unique<Chunk>();



    for (int x = 0; x <= 15; ++x) {
        for (int z = 0; z <= 15; ++z) {
            int world_x = result.chunk_pos.x * 16 + x;
            int world_z = result.chunk_pos.z * 16 + z;

            float rawTemperature = temperature.GetNoise((float)world_x,(float)world_z );
            float rawHumidity = humidity.GetNoise((float)world_x,(float)world_z );

            float stretchFactor = 1.8f;
            rawTemperature = std::clamp(rawTemperature * stretchFactor, -1.0f, 1.0f);
            rawHumidity    = std::clamp(rawHumidity * stretchFactor, -1.0f, 1.0f);

            result.chunk->m_blockTemperatures[x][z] = (rawTemperature + 1.0f) * 0.5f;
            result.chunk->m_blockHumidities[x][z] = (rawHumidity + 1.0f) * 0.5f;

            float val_macro = noise_macro.GetNoise((float)world_x, (float)world_z);
            float val_micro = noise_micro.GetNoise((float)world_x, (float)world_z);

            float shaped_macro = val_macro - std::abs(val_micro) * 0.3f;
            int base_height = 160;
            int surface_y = base_height + (int)(shaped_macro * 60.0f) + (int)(val_micro * 6.0f);
            constexpr int SAND_LEVEL = 110;
            constexpr int MOUNTAIN_LEVEL = 190;
            constexpr int WATER_LEVEL = 116;

            for (int y = 0; y <= 15; ++y) {
                int world_y = result.chunk_pos.y * 16 + y;
                unsigned short current_material = m_blockRegistry.getIDByName("AIR");

                if (world_y > surface_y){
                    if (world_y < WATER_LEVEL){
                        current_material = m_blockRegistry.getIDByName("WATER");
                    } else{
                        current_material = m_blockRegistry.getIDByName("AIR");
                    }
                }
                else if (world_y == surface_y){
                    if (world_y < SAND_LEVEL){
                        current_material = m_blockRegistry.getIDByName("SAND");

                    } else if (surface_y > MOUNTAIN_LEVEL){
                        current_material = m_blockRegistry.getIDByName("STONE");
                    } else{
                        current_material = m_blockRegistry.getIDByName("GRASS_BLOCK");
                        if (world_y < WATER_LEVEL - 1){
                            current_material = m_blockRegistry.getIDByName("DIRT");
                        }
                    }
                }
                else if (world_y > surface_y -4){
                    if (surface_y < SAND_LEVEL){
                        current_material = m_blockRegistry.getIDByName("SAND");
                    } else if (surface_y > MOUNTAIN_LEVEL){
                        current_material = m_blockRegistry.getIDByName("STONE");
                    } else{
                        current_material = m_blockRegistry.getIDByName("DIRT");
                    }
                } else{
                        current_material = m_blockRegistry.getIDByName("STONE");

                }

                if (current_material != m_blockRegistry.getIDByName("AIR")) {

                    float mask_val = noise_cave_mask.GetNoise((float)world_x, (float)world_y, (float)world_z);

                    if (mask_val > 0.4f) {

                        float cave_val = noise_caves.GetNoise((float)world_x, (float)world_y, (float)world_z);

                        float base_threshold = 0.12f;

                        float carve_threshold = base_threshold * mask_val;

                        int depth_from_surface = surface_y - world_y;
                        if (depth_from_surface <= 5) {
                            carve_threshold += (5 - depth_from_surface) * 0.008f;
                        }

                        if (std::abs(cave_val) < carve_threshold and m_blockRegistry.getBlockByID(current_material).isSolid) {
                            current_material = m_blockRegistry.getIDByName("AIR");
                        }
                    }
                }

                if (world_y == 1){
                    current_material = m_blockRegistry.getIDByName("STONE");

                }

                result.chunk->setBlock(x, y, z, current_material);
            }
        }
    }

    queue_generation_result.push(std::move(result));
}
