#include "utils.h"
#include <fstream>
#include <iostream>

char* readFile(const std::string &fileName) {

    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Failed to open file!" << std::endl;
    }

    size_t fileSize = (size_t) file.tellg();
    char* buffer = new char[fileSize];

    file.seekg(0);
    file.read(buffer, fileSize);

    file.close();

    return buffer;
}
