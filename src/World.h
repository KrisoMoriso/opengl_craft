#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Chunk.h"
#include "ThreadPool.h"
class Renderer;

class World {
public:
    struct BLOCK_MATERIALS{
        static constexpr unsigned short AIR = 0;
        static constexpr unsigned short DIRT = 1;
        static constexpr unsigned short GRASS_BLOCK = 2;
        static constexpr unsigned short STONE = 3;
        static constexpr unsigned short OAK_PLANKS = 4;
        static constexpr unsigned short OAK_LOG = 5;
        static constexpr unsigned short SAND = 6;
        static constexpr unsigned short WATER = 7;
        static std::string match_material(unsigned short material){
            if (material == 0){return "AIR";}
            if (material == 1){return "DIRT";}
            if (material == 2) {return "GRASS_BLOCK";}
            if (material == 3) {return "STONE";}
            if (material == 4) {return "OAK_PLANKS";}
            if (material == 5) {return "OAK_LOG";}
            if (material == 6) {return "SAND";}
            return "UNKNOWN";
        }
        static bool is_solid(unsigned short material){
           if (material == 1 or material == 2 or material == 3 or material == 4 or material == 5 or material == 6){
               return true;
           }
           return false;
        }
        static bool is_transparent(unsigned short material){
            if (material == 7){
                return true;
            }
            return false;
        }
    };
    struct ChunkPos{
        int x, y, z;
        bool operator==(const ChunkPos& other) const{
            return x == other.x and y == other.y and z == other.z;
        }
    };
    struct ChunkPosHash{
        std::size_t operator()(const ChunkPos& pos) const{
            std::size_t hash = 0;
            auto hash_combine = [](std::size_t& hash_, int position){
              hash_ ^= std::hash<int>{}(position) + 0x9e3779b9 + (hash_ << 6) + (hash_ >> 2);
            };
            hash_combine(hash, pos.x);
            hash_combine(hash, pos.y);
            hash_combine(hash, pos.z);
            return hash;
        }
    };
    std::unordered_map<ChunkPos, std::shared_ptr<Chunk>, ChunkPosHash> m_chunks;
    static constexpr int WORLD_CHUNK_HEIGHT = 16;
    World(ThreadPool& threadPool, Renderer& renderer, int seed, int renderDistance);
    static ChunkPos get_chunk_position(glm::vec3 position);
    unsigned short get_block_material(int x, int y, int z);
    void set_block_material(int x, int y, int z, unsigned short material);
    void generate_world(glm::vec3 player_position);
private:
    ThreadPool& m_thread_pool;
    Renderer& m_renderer;
    int m_seed;
    int m_render_distance;
    ChunkPos m_last_player_chunk = {99999,99999,99999};
    struct GenerationJob{
        ChunkPos chunk_pos;
    };
    struct GenerationResult{
        ChunkPos chunk_pos;
        std::unique_ptr<Chunk> chunk;
    };
    void generate_chunk(GenerationJob generation_job, ThreadPool::SafeQueue<GenerationResult>& queue_generation_result);
    ThreadPool::SafeQueue<GenerationResult> m_queue_generation_result;
    std::vector<ChunkPos> m_queue_to_generate;
    std::vector<ChunkPos> m_queue_to_delete;
    std::unordered_set<ChunkPos, ChunkPosHash> m_chunks_in_progress;
};
