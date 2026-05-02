#pragma once
#include <glad/glad.h>
#include <array>
#include <shared_mutex>

#include "Block.h"
#include <glm/glm.hpp>


class Chunk {
public:
    std::array<Block, 4096> m_blocks;
    Chunk();
    unsigned short getBlockMaterial(glm::vec3 position);//position relative to the chunk
    bool m_is_meshing = false;
    bool m_is_generating = false;
    void setBlock(int x, int y, int z, unsigned short material);
    unsigned short getBlockMaterial(int x, int y, int z);
    std::shared_mutex m_block_mutex;
};
