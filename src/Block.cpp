#include "Block.h"

Block::Block(int material_type)
{
    m_material_type = material_type;
}

Block::Block()
{
    m_material_type = 0; //air
}