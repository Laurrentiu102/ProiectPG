#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN };

    class Camera
    {
    public:
        Camera() = default;
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        glm::mat4 getViewMatrix();
        void move(MOVE_DIRECTION direction);
        void rotate(float pitch, float yaw);
        void clampAngle(float& angle, float lower, float upper);
        void mouseCallback(double xpos, double ypos);

    public:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;

        glm::vec3 originalCameraUpDirection;
        glm::vec3 originalCameraFrontDirection;

        float lastX = 0;
        float lastY = 0;
        bool firstTimeInFrame = true;
        float sensitivity = 0.05f;
        float yaw = 0.0f;
        float pitch = 0.0f;

        float speed = 0.06f;
    };

}

#endif