#include "ResourceManager.h"

#include <fstream>
#include <sstream>

std::string ResourceManager::loadFileIntoString(const char* filePath){
    std::string file_contents;
    std::ifstream file(filePath);
    if (file.fail()){
        printf("failed to open file %s", filePath);
        return file_contents;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    file_contents = buffer.str();
    file.close();
    return file_contents;
}
