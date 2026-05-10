#pragma once
#include "glad/glad.h"
#include <string>
#include <unordered_map>
#include <vector>


struct BlockDefinition{
    std::string name = "DEFAULTBLOCKNAME";
    bool isSolid = true, isTransparent = false;
    GLuint faceTextureLayers[6] = {0};
};

class BlockRegistry {
public:
    BlockDefinition getBlockByID(unsigned short ID);
    unsigned short getIDByName(const std::string& blockName);
    void loadFromJSON(const std::string &JSONFileName);
    std::vector<std::string> textureLayerFileNames;
private:
    std::unordered_map<unsigned short, BlockDefinition> m_blockDefinitions;
    std::unordered_map<std::string, unsigned short> m_nameToID;
    GLuint getOrAddTexture(const std::string& texturePath);
};
