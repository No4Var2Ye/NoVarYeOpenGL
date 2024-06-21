#ifndef CAMERA_H
#define CAMERA_H

#include <GLAD/glad.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>

// 定义相机移动的抽象方向枚举
// 兼容性
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认相机参数
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.0f;        // 移动速度
const float SENSITIVITY = 0.08f; // 鼠标灵敏度
const float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 Position; // 位置
    glm::vec3 Front;    // 前向向量
    glm::vec3 Up;       // 上向向量
    glm::vec3 Right;    // 右向向量
    glm::vec3 WorldUp;  // 世界坐标系中的上向向量

    // Euler Angles
    float Yaw;   // 偏航角
    float Pitch; // 俯仰角

    // Camera options
    float MovementSpeed;    // 移动速度
    float MouseSensitivity; // 鼠标灵敏度
    float Zoom;             // 缩放

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 返回视图View矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Input
    // 接受以相机定义的ENUM形式的输入参数
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // 鼠标输入系统的输入
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // 确保俯仰角在合理范围内，避免屏幕翻转
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // 鼠标滚轮事件
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    void resetToDefaults(glm::vec3 cameraInitPos = glm::vec3(0.0f, 0.0f, 4.0f))
    {
        Position = cameraInitPos;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = -90.0f;
        Pitch = 0.0f;
        Front = glm::vec3(0.0f, 0.0f, -1.0f);
        updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif // CAMERA_H
