#include "rendering/Camera.h"

Mat4x4 Camera::getMatView() {
    Vec3 vTarget = position + front;
    return MatrixLookAt(position, vTarget, up);
}

Mat4x4 Camera::getMatProj() {
    return MatrixProjection(fov, aspectRatio, nearPlane, farPlane);
}

Mat4x4 Camera::getMatViewProj() {
    return getMatProj() * getMatView();
}

void Camera::processKeyboardInput(const std::string& direction, float deltaTime, float speed) {
    float velocity = speed * deltaTime;
    if (direction == "FORWARD") {
        position.x -= (cosf(yaw) * velocity);
        position.z -= (sinf(yaw) * velocity);
    }   
    if (direction == "BACKWARD") {
        position.x += (cosf(yaw) * velocity);
        position.z += (sinf(yaw) * velocity);
    }
    if (direction == "LEFT")
        position -= (right * velocity);
    if (direction == "RIGHT")
        position += (right * velocity);
    if (direction == "UP")
        position.y += velocity;
    if (direction == "DOWN")
        position.y -= velocity;

    isDirty = true;
}

void Camera::processMouseInput(float xOffset, float yOffset, float sensitivity) {
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw -= xOffset;
    pitch += yOffset;

    // Constrain pitch to avoid gimbal lock
    if (pitch > PI / 2.01f) pitch = PI / 2.01f;
    if (pitch < -PI / 2.01f) pitch = -PI / 2.01f;

    // Update camera vectors
    updateCameraVectors();

    isDirty = true;
}

void Camera::updateCameraVectors() {
    // Calculate the new front vector
    Vec3 newFront;
    newFront.x = cosf(yaw) * cosf(pitch);
    newFront.y = sinf(pitch);
    newFront.z = sinf(yaw) * cosf(pitch);

    // adjust to prevent floating point errors
    const float epsilon = 0.001;
    if (abs(newFront.x) < epsilon) {
        newFront.x = newFront.x < 0 ? -epsilon : epsilon;
    }
    if (abs(newFront.y) < epsilon) {
        newFront.y = newFront.y < 0 ? -epsilon : epsilon;
    }
    if (abs(newFront.z) < epsilon) {
        newFront.z = newFront.z < 0 ? -epsilon : epsilon;
    }

    front = normalise(newFront);

    // Recalculate right and up vectors
    right = cross(front, worldUp);
    right = normalise(right);

    up = cross(right, front);
    up = normalise(up);
}
