
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <NoVarYe/shader.h>
#include <NoVarYe/camera.h>

#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings 设置
const unsigned int SCR_WIDTH = 1200; // 窗口宽度
const unsigned int SCR_HEIGHT = 900; // 窗口高度

// stores how much we're seeing of either texture
float mixValue = 0.2f;

// camera
glm::vec3 cameraInitPos = glm::vec3(0.0f, 0.0f, 3.0f);
Camera camera(cameraInitPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
// 帧差
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(0.0f, 1.0f, 4.0f);

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
    Shader customizedShader("shaders/10/shader.vs", "shaders/10/shader_multiple_lights.fs");
    Shader lightSourceShader("shaders/light_source/light_source.vs", "shaders/light_source/light_source.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)};

    // positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)};

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 将顶点数据复制到缓冲区对象中
    // GL_STATIC_DRAW 指定数据不会或几乎不会改变
    // GLL_DYNAMIC_DRAW 指定数据会被改变很多
    // GL_STREAM_DRAW 指定数据每次绘制时都会改变
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // light source
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // load and create a texture
    // -------------------------
    unsigned int diffuseMap = loadTexture("assets/textures/EYE.png");
    unsigned int specularMap = loadTexture("assets/textures/EYE_specular.png");
    unsigned int emissionMap = loadTexture("assets/textures/EYE_emmsion.png");

    // shader configuration
    // --------------------
    customizedShader.use();
    customizedShader.setInt("material.diffuse", 0);
    customizedShader.setInt("material.specular", 1);
    customizedShader.setInt("material.emission", 2);

    // 解绑缓冲区和顶点数组对象，防止误操作
    // OpenGL不会绑定ID为0的缓冲对象到任何目标上
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    customizedShader.use(); // don't forget to activate/use the shader before setting uniforms!
    // glUniform1i(glGetUniformLocation(customizedShader.ID, "texture1"), 0);
    // customizedShader.setInt("texture1", 0);
    // customizedShader.setInt("texture2", 1);

    // pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
    // -----------------------------------------------------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    customizedShader.setMat4("projection", projection);

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
        glClearColor(0.23f, 0.23f, 0.23f, 1.0f);
        // GL_COLOR_BUFFER_BIT: 指定颜色缓冲区
        // GL_DEPTH_BUFFER_BIT: 指定深度缓冲区
        // GL_STENCIL_BUFFER_BIT: 指定模板缓冲区
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        customizedShader.setFloat("mixValue", mixValue);

        // be sure to activate shader when setting uniforms/drawing objects
        customizedShader.use();
        customizedShader.setVec3("viewPos", camera.Position);
        customizedShader.setFloat("material.shininess", 32.0f);

        // directional light
        customizedShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        customizedShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        customizedShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        customizedShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        customizedShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        customizedShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        customizedShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        customizedShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        customizedShader.setFloat("pointLights[0].constant", 1.0f);
        customizedShader.setFloat("pointLights[0].linear", 0.09f);
        customizedShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        customizedShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        customizedShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        customizedShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        customizedShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        customizedShader.setFloat("pointLights[1].constant", 1.0f);
        customizedShader.setFloat("pointLights[1].linear", 0.09f);
        customizedShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        customizedShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        customizedShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        customizedShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        customizedShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        customizedShader.setFloat("pointLights[2].constant", 1.0f);
        customizedShader.setFloat("pointLights[2].linear", 0.09f);
        customizedShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        customizedShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        customizedShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        customizedShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        customizedShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        customizedShader.setFloat("pointLights[3].constant", 1.0f);
        customizedShader.setFloat("pointLights[3].linear", 0.09f);
        customizedShader.setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        customizedShader.setVec3("spotLight.position", camera.Position);
        customizedShader.setVec3("spotLight.direction", camera.Front);
        customizedShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        customizedShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        customizedShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        customizedShader.setFloat("spotLight.constant", 1.0f);
        customizedShader.setFloat("spotLight.linear", 0.09f);
        customizedShader.setFloat("spotLight.quadratic", 0.032f);
        customizedShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        customizedShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        customizedShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        customizedShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        customizedShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        // bind emission map
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap);

        // render boxes
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            if (i % 3 == 0)
            {
                angle = glfwGetTime() * 25.0f;
            }
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            customizedShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // also draw the lamp object(s)
        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
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
    glDeleteVertexArrays(1, &cubeVAO);
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
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}