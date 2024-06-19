
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>

int main()
{
    glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 trans = glm::mat4(1.0f);
    
    trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
    
    vec = trans * vec;
    std::cout << vec.x << vec.y << vec.z << std::endl;
    std::cout.flush();

    std::cout << "Press any key to exit...";
    std::cin.get();
    return 0;
}
