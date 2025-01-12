#include "camera.h"

Camera::Camera(glm::vec3 cameraPosition)
{
    this->cameraPosition = cameraPosition;
    this->cameraViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
    this->rotationOx = 0.0f;
    this->rotationOy = -90.0f;
}

Camera::Camera()
{
    this->cameraPosition = glm::vec3(0.0f, 0.0f, 100.0f);
    this->cameraViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
    this->rotationOx = 0.0f;
    this->rotationOy = -90.0f;
}

Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraViewDirection, glm::vec3 cameraUp)
{
    this->cameraPosition = cameraPosition;
    this->cameraViewDirection = cameraViewDirection;
    this->cameraUp = cameraUp;
    this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
}

Camera::~Camera()
{
}

void Camera::keyboardMoveFront(float cameraSpeed)
{
    cameraPosition += cameraViewDirection * cameraSpeed;
}

void Camera::keyboardMoveBack(float cameraSpeed)
{
    cameraPosition -= cameraViewDirection * cameraSpeed;
}

void Camera::keyboardMoveLeft(float cameraSpeed)
{
    glm::vec3 rightDirection = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
    cameraPosition -= rightDirection * cameraSpeed;
}

void Camera::keyboardMoveRight(float cameraSpeed)
{
    glm::vec3 rightDirection = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
    cameraPosition += rightDirection * cameraSpeed;
}

void Camera::keyboardMoveUp(float cameraSpeed)
{
    cameraPosition += cameraUp * cameraSpeed;
}

void Camera::keyboardMoveDown(float cameraSpeed)
{
    cameraPosition -= cameraUp * cameraSpeed;
}

void Camera::rotateOx(float angle)
{
    rotationOx += angle;//keep track of rotation
    updateCameraVectors();
}

void Camera::rotateOy(float angle)
{
    rotationOy += angle;//keep track of rotation
    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
    front.y = sin(glm::radians(rotationOx));
    front.z = sin(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
    cameraViewDirection = glm::normalize(front);
    // Also re-calculate the right and up vector
    cameraRight = glm::normalize(glm::cross(cameraViewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraViewDirection));
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(cameraPosition, cameraPosition + cameraViewDirection, cameraUp);
}

glm::vec3 Camera::getCameraPosition()
{
    return cameraPosition;
}

glm::vec3 Camera::getCameraViewDirection()
{
    return cameraViewDirection;
}

glm::vec3 Camera::getCameraUp()
{
    return cameraUp;
}

void Camera::setPosition(glm::vec3 position)
{
    cameraPosition = position;
}

void Camera::setRotation(float pitch, float yaw) 
{
    rotationOx = pitch;
    rotationOy = yaw;
    updateCameraVectors();
}

void Camera::rotate(float pitch, float yaw) {
    rotationOx += pitch;
    rotationOy += yaw;
    updateCameraVectors();
}

glm::vec3 Camera::getCameraRightDirection()
{
    return glm::normalize(glm::cross(cameraViewDirection, cameraUp));
}
