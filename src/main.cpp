#include <vulkan/vulkan.h>
#include <glm/vec4.hpp> // glm::vec4
#include <iostream>

int main() {

    VkInstance instance;
    glm::vec4 vec;
    vec.x = 10;
    vec.y = 11;
    vec.z = 12;
    vec.a = 13;
  
    std::cout << "( x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " )" << std::endl;
    
    return 0;
}