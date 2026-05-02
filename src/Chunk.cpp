#include <glad/glad.h>
#include "Chunk.h"
#include "World.h"
#include <glm/glm.hpp>
Chunk::Chunk() {
    for (int x = 0; x <= 15; ++x) {
        for (int y = 0; y <= 15; ++y) {
            for (int z = 0; z <= 15; ++z) {
                m_blocks[z + y * 16 + x*256] = Block( World::BLOCK_MATERIALS::STONE );
            }
        }
    }
}

unsigned short Chunk::getBlockMaterial(glm::vec3 position){

    return m_blocks[position.x*256 + position.y*16 + position.z].m_material_type;

}

void Chunk::setBlock(int x, int y, int z, unsigned short material){
    if (!(x >= 0 and x <=15 and y >= 0 and y <=15 and z >= 0 and z <=15)) return;
    m_blocks[z + y * 16 + x*256] = Block( material );
}

unsigned short Chunk::getBlockMaterial(int x, int y, int z){
    if (!(x >= 0 and x <=15 and y >= 0 and y <=15 and z >= 0 and z <=15)) return 10000;
    return m_blocks[z + y * 16 + x*256].m_material_type;
}




