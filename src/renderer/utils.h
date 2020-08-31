#ifndef UTILS_H
#define UTILS_H

#include <string>

struct FileContents {
    char* p_byteCode;
    uint32_t fileSize;
};

// Utility function used to read a file name (usually a shader).
FileContents readFile(const std::string &fileName); 

#endif
