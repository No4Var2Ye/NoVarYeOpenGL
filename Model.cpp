
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "NoVarYe/shader.h"
#include "NoVarYe/camera.h"
#include "NoVarYe/model.h"
#include "NoVarYe/vertex.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings 设置
const unsigned int SCR_WIDTH = 1200; // 窗口宽度
const unsigned int SCR_HEIGHT = 900; // 窗口高度

// stores how much we're seeing of either texture
float mixValue = 0.2f;

// camera
glm::vec3 cameraInitPos = glm::vec3(0.0f, 15.0f, 60.0f);
Camera camera(cameraInitPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
// 帧差
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(0.0f, 30.0f, 30.0f);

// 模型路径
const char *modelPath = "assets/models/ANBY/anby.obj";
// const char *modelPath = "assets/models/lly/scene.obj";
// Model ourModel("assets/models/lly/scene.obj");
// Model ourModel("assets/models/cup/cup.obj");
// Model ourModel("assets/models/nanosuit/nanosuit.obj");

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// 兼容性配置
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建窗口对象
    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NoVarYeOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 鼠标指针处理模式
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 加载所有OpenGL函数指针
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shaders/loading/model_loading.vs", "shaders/loading/model_loading.fs");
    Shader lightSourceShader("shaders/light_source/light_source.vs", "shaders/light_source/light_source.fs");

    // load models
    // -----------
    Model ourModel(modelPath);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    std::vector<float> vertices = generateCubeVertices();
    const float *vertexDataPtr = static_cast<const float *>(vertices.data());

    // positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(20.0f, 20.0f, 20.0f),
        glm::vec3(20.0f, 20.0f, -20.0f),
        glm::vec3(-20.0f, 20.0f, 20.0f),
        glm::vec3(-20.0f, 20.0f, -20.0f)};

    // light source
    unsigned int VBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertexDataPtr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 解绑缓冲区和顶点数组对象，防止误操作
    // OpenGL不会绑定ID为0的缓冲对象到任何目标上
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // render loop 渲染循环
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 键盘输入
        // -----
        processInput(window);

        // 开始渲染
        // ------
        // glClearColor(0.23f, 0.23f, 0.23f, 1.0f);
        glClearColor(0.75f, 0.52f, 0.3f, 1.0f);
        // glClearColor(1.0f, 0.7f, 0.9f,0.82f);
        // GL_COLOR_BUFFER_BIT: 指定颜色缓冲区
        // GL_DEPTH_BUFFER_BIT: 指定深度缓冲区
        // GL_STENCIL_BUFFER_BIT: 指定模板缓冲区
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ourShader.setFloat("mixValue", mixValue);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);

        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        glm::mat4 projection2 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view2 = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection2);
        ourShader.setMat4("view", view2);

        // render the loaded model
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        // model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        ourShader.setMat4("model", model);

        float radius = 20.0f;
        float angleOffet = glm::radians(90.0f);
        float offsetX0 = static_cast<float>(cos(glfwGetTime() + angleOffet * 0)) * radius;
        float offsetZ0 = static_cast<float>(sin(glfwGetTime() + angleOffet * 0)) * radius;
        float offsetX1 = static_cast<float>(cos(glfwGetTime() + angleOffet * 1)) * radius;
        float offsetZ1 = static_cast<float>(sin(glfwGetTime() + angleOffet * 1)) * radius;
        float offsetX2 = static_cast<float>(cos(glfwGetTime() + angleOffet * 2)) * radius;
        float offsetZ2 = static_cast<float>(sin(glfwGetTime() + angleOffet * 2)) * radius;
        float offsetX3 = static_cast<float>(cos(glfwGetTime() + angleOffet * 3)) * radius;
        float offsetZ3 = static_cast<float>(sin(glfwGetTime() + angleOffet * 3)) * radius;

        glm::vec3 lightPos0 = glm::vec3(offsetX0, pointLightPositions[0].y, offsetZ0);
        glm::vec3 lightPos1 = glm::vec3(offsetX1, pointLightPositions[1].y, offsetZ1);
        glm::vec3 lightPos2 = glm::vec3(offsetX2, pointLightPositions[2].y, offsetZ2);
        glm::vec3 lightPos3 = glm::vec3(offsetX3, pointLightPositions[3].y, offsetZ3);

        // directional light
        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ourShader.setVec3("dirLight.ambient", 0.3f, 0.24f, 0.14f);
        ourShader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        ourShader.setVec3("pointLights[0].position", lightPos0);
        ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09f);
        ourShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        ourShader.setVec3("pointLights[1].position", lightPos1);
        ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[1].constant", 1.0f);
        ourShader.setFloat("pointLights[1].linear", 0.09f);
        ourShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        ourShader.setVec3("pointLights[2].position", lightPos2);
        ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[2].constant", 1.0f);
        ourShader.setFloat("pointLights[2].linear", 0.09f);
        ourShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        ourShader.setVec3("pointLights[3].position", lightPos3);
        ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[3].constant", 1.0f);
        ourShader.setFloat("pointLights[3].linear", 0.09f);
        ourShader.setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        ourShader.setVec3("spotLight.position", camera.Position);
        ourShader.setVec3("spotLight.direction", camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.8f, 0.0f, 0.8f);
        ourShader.setVec3("spotLight.diffuse", 0.8f, 0.0f, 0.8f);
        ourShader.setVec3("spotLight.specular", 1.0f, 0.9f, 1.0f);
        ourShader.setFloat("spotLight.constant", 1.0f);
        ourShader.setFloat("spotLight.linear", 0.09f);
        ourShader.setFloat("spotLight.quadratic", 0.032f);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(13.0f)));

        ourModel.Draw(ourShader);

        // also draw the lamp object(s)
        lightSourceShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            float angleOffet = glm::radians(90.0f * i);
            float offsetX = static_cast<float>(sin(glfwGetTime() + angleOffet)) * radius;
            float offsetZ = static_cast<float>(cos(glfwGetTime() + angleOffet)) * radius;
            glm::vec3 lightPosi = glm::vec3(offsetX, pointLightPositions[i].y * (1 + 0.3 * sin(glfwGetTime() * 0.4)), offsetZ);
            model = glm::translate(model, lightPosi);
            model = glm::scale(model, glm::vec3(4.0f));
            lightSourceShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 交换缓冲区并处理IO事件`
        // glfw: swap buffers and poll IO events
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 在资源不再使用时释放所有资源：
    // de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // 终止，清理所有之前分配的GLFW资源
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// 处理输入函数：查询GLFW是否按下或释放相关键，并作出相应反应
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    // Esc
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 检测 F3 按键是否按下
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
    {
        // 获取当前的多边形模式
        GLint polygonMode;
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode);

        // 切换多边形模式
        if (polygonMode == GL_LINE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 切换为填充模式
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 切换为线框模式
    }

    // up and down
    // Adjust mixValue
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }

    // WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    // if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        camera.resetToDefaults(cameraInitPos);
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 当窗口大小改变时调用此回调函数
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions;
    // note that width and height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
