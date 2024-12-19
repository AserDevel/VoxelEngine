#pragma once

#include <iostream>
#include "utilities/standard.h"

class Camera {
public:
    Vec3 position;       // Camera position
    Vec3 front;          // Direction the camera is facing
    Vec3 up;             // Up vector
    Vec3 right;          // Right vector (calculated)
    Vec3 worldUp;        // Reference up vector

    float yaw;           // Horizontal rotation
    float pitch;         // Vertical rotation
    float fov;           // Field of view
    float aspectRatio;   // Aspect ratio of the viewport
    float nearPlane;     // Near clipping plane
    float farPlane;      // Far clipping plane

    // Constructor
    Camera(Vec3 startPosition, Vec3 startUp, float startYaw, float startPitch, 
           float startFOV, float startAspect, float startNear, float startFar)
        : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch),
          fov(startFOV), aspectRatio(startAspect), nearPlane(startNear), farPlane(startFar) {
        
        updateCameraVectors();
    }

    Mat4x4 getMatView();

    Mat4x4 getMatProj();

    Mat4x4 getMatCamera();

    void processKeyboardInput(const std::string& direction, float deltaTime, float speed);

    void processMouseInput(float xOffset, float yOffset, float sensitivity = 0.1f);

private:
    void updateCameraVectors();
};

