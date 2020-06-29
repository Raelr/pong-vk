#include "utils.h"
#include <fstream>
#include <iostream>

// Simple utility function for returnng the contents of a file.
FileContents readFile(const std::string &fileName) {
    // Get the contents of the file and interpret it as a byte 
    // array
    
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    // Make sure we've actually opened the file
    if (!file.is_open()) {
        std::cout << "Failed to open file!" << std::endl;
    }
    
    FileContents contents;
    // Create an array with the size of this file into it.
    size_t fileSize = (size_t) file.tellg();
    char* buffer = new char[fileSize];

    file.seekg(0);
    // Pass the file contents to the array
    file.read(buffer, fileSize);
    // Close the file
    file.close();
    //return file contents
    contents.p_byteCode = buffer;
    contents.fileSize = fileSize;

    return contents;
}
