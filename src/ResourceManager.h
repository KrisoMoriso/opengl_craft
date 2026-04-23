#pragma once
#include <string>


class ResourceManager {
public:
    static std::string loadFileIntoString(const char* filePath);
};
