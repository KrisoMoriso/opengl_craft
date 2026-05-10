#include "BlockRegistry.h"
#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

GLuint BlockRegistry::getOrAddTexture(const std::string& texturePath) {
    for (int i = 0; i < textureLayerFileNames.size(); i++) {
        if (textureLayerFileNames[i] == texturePath) {
            return i;
        }
    }
    textureLayerFileNames.push_back(texturePath);
    return textureLayerFileNames.size() - 1;
}

BlockDefinition BlockRegistry::getBlockByID(unsigned short ID){
    BlockDefinition defaultDefinition = {};
    if (m_blockDefinitions.contains(ID)){
        return m_blockDefinitions[ID];
    }
    return defaultDefinition;
}

unsigned short BlockRegistry::getIDByName(const std::string& blockName){
    if (m_nameToID.contains(blockName)){
        return m_nameToID[blockName];
    }
    return 0;
}

void BlockRegistry::loadFromJSON(const std::string& filepath) {
    //manually add grass side overlay
    getOrAddTexture("../textures/block/grass_block_side_overlay.png");

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return;
    }
    json jsonData;
    file >> jsonData;
    for (auto& [key, value] : jsonData.items()) {
        unsigned short materialID = std::stoi(key);
        BlockDefinition definition;
        definition.name = value["name"];
        definition.isSolid = value["isSolid"];
        definition.isTransparent = value["isTransparent"];
        if (!value["textures"].is_null()) {
            auto textures = value["textures"];
            if (textures.contains("all")) {
                GLuint layerId = getOrAddTexture(textures["all"]);
                for (int i = 0; i < 6; i++){
                    definition.faceTextureLayers[i] = layerId;
                }
            }
            else {
                if (textures.contains("top")) {
                    definition.faceTextureLayers[4] = getOrAddTexture(textures["top"]);
                }
                if (textures.contains("bottom")) {
                    definition.faceTextureLayers[5] = getOrAddTexture(textures["bottom"]);
                }
                if (textures.contains("sides")) {
                    GLuint sideLayerId = getOrAddTexture(textures["sides"]);
                    definition.faceTextureLayers[0] = sideLayerId;
                    definition.faceTextureLayers[1] = sideLayerId;
                    definition.faceTextureLayers[2] = sideLayerId;
                    definition.faceTextureLayers[3] = sideLayerId;
                }
            }
        }
        m_blockDefinitions[materialID] = definition;
        m_nameToID[definition.name] = materialID;
    }
}
