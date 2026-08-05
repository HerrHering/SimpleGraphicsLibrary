/**
 * @file Camera.hpp
 * @brief Free-look 3D Euler Angle camera system for FPS-style navigation.
 *
 * @details
 * ASSUMPTIONS:
 * - Uses Right-Handed Coordinate system (Standard OpenGL convention: +X Right, +Y Up, -Z Forward).
 * - Yaw initialized at -90.0f points look-vector towards -Z axis.
 * - Pitch clamped between [-89.0f, +89.0f] to avoid gimbal lock flip.
 *
 * @example Usage Example:
 * @code
 * #include "Camera.hpp"
 *
 * Camera camera(glm::vec3(0.0f, 2.0f, 5.0f)); // Start position
 *
 * void frameLoop(float deltaTime, float mouseDx, float mouseDy) {
 *     // Rotate view from mouse delta offsets
 *     camera.rotate(mouseDx, mouseDy);
 *
 *     // Move camera along move direction vectors
 *     camera.move(Camera::FORWARD, deltaTime);
 *
 *     // Retrieve look-at view matrix to pass into GLSL shader uniforms
 *     glm::mat4 viewMatrix = camera.getViewMatrix();
 * }
 * @endcode
 */

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @class Camera
 * @brief A standard 3D Euler angle FPS free-cam implementation.
 */
class Camera {
public:
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float speed;
    float sensitivityX;
    float sensitivityY;

    explicit Camera(glm::vec3 startPos = glm::vec3(0.0f, 2.0f, 5.0f), 
                    glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f), 
                    float startYaw = -90.0f, 
                    float startPitch = -15.0f,
                    float sensX = 0.1f,
                    float sensY = 0.08f
                )
        : position(startPos), worldUp(startUp), yaw(startYaw), pitch(startPitch), speed(5.0f), sensitivityX(sensX), sensitivityY(sensY) {
        updateCameraVectors();
    }

    void move(Movement direction, float deltaTime) {
        const float velocity = speed * deltaTime;
        if (direction == FORWARD)  position += front * velocity;
        if (direction == BACKWARD) position -= front * velocity;
        if (direction == LEFT)     position -= right * velocity;
        if (direction == RIGHT)    position += right * velocity;
        if (direction == UP)       position += worldUp * velocity;
        if (direction == DOWN)     position -= worldUp * velocity;
    }

    void rotate(float xOffset, float yOffset) {
        yaw += xOffset * sensitivityX;
        pitch += yOffset * sensitivityY;

        if (pitch > 89.0f)  pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateCameraVectors();
    }

    [[nodiscard]] glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);
        
        right = glm::normalize(glm::cross(front, worldUp));
        up    = glm::normalize(glm::cross(right, front));
    }
};