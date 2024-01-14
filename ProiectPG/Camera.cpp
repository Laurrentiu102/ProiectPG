#include "Camera.hpp"

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(glm::vec3(cameraTarget - cameraPosition));
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));

        this->originalCameraUpDirection = cameraUp;
        this->originalCameraFrontDirection = this->cameraFrontDirection;
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    void Camera::move(MOVE_DIRECTION direction) {
        if (direction == MOVE_FORWARD)
            cameraPosition += cameraFrontDirection * speed;

        if (direction == MOVE_BACKWARD)
            cameraPosition -= cameraFrontDirection * speed;

        if (direction == MOVE_UP)
            cameraPosition += glm::vec3(0.0f, 1.0f, 0.0f) * speed;

        if (direction == MOVE_DOWN)
            cameraPosition -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;

        if (direction == MOVE_LEFT)
            cameraPosition -= cameraRightDirection * speed;

        if (direction == MOVE_RIGHT)
            cameraPosition += cameraRightDirection * speed;

        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    glm::mat3 E(float h, float p, float r)
    {
        return glm::rotate(glm::mat4(1.0f), glm::radians(h), glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::rotate(glm::mat4(1.0f), glm::radians(p), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void Camera::rotate(float pitch, float yaw) {
        glm::mat3 euler = E(yaw, pitch, 0.0f);

        cameraFrontDirection = glm::normalize(euler * originalCameraFrontDirection);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, originalCameraUpDirection));
        cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    void Camera::clampAngle(float& angle, float lower, float upper)
    {
        angle > upper ? angle = upper : angle;
        angle < lower ? angle = lower : angle;
    }
    void Camera::mouseCallback(double xpos, double ypos)
    {
        if (firstTimeInFrame)
        {
            lastX = (float)xpos;
            lastY = (float)ypos;
            firstTimeInFrame = false;
        }
        float deltaX = (float)xpos - lastX;
        float deltaY = (float)ypos - lastY;

        lastX = (float)xpos;
        lastY = (float)ypos;

        yaw -= deltaX * sensitivity;
        pitch -= deltaY * sensitivity;

        clampAngle(pitch, -70.0f, 89.0f);

        rotate(pitch, yaw);
    }
}