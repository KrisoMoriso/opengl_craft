#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "BlockRegistry.h"
#include "Chunk.h"
#include "ThreadPool.h"
class Renderer;

class World {
public:
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
    World(ThreadPool& threadPool, Renderer& renderer, int seed, int renderDistance, BlockRegistry &blockRegistryInstance);
    static ChunkPos get_chunk_position(glm::vec3 position);
    unsigned short get_block_material(int x, int y, int z);
    void set_block_material(int x, int y, int z, unsigned short material);
    void generate_world(glm::vec3 player_position);
private:
    ThreadPool& m_thread_pool;
    Renderer& m_renderer;
    BlockRegistry &m_blockRegistry;
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
