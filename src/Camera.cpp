#include <cmath>
#include "Camera.hpp"
#include "../../src/Logger/Logger.hpp"

Camera::Camera(float pos_X, float pos_Y, float pos_Z, float up_X, float up_Y, float up_Z, float yaw, float pitch) {
	MyglobalLogger().logMessage(Logger::INFO, "Start camera.", __FILE__, __LINE__);  // Исправлено: убрали лишние *
	Position = glm::vec3(pos_X, pos_Y, pos_Z);
	WorldUp = glm::vec3(up_X, up_Y, up_Z);
	Yaw = yaw;
	Pitch = pitch;
	Front = glm::vec3(0.0f, 0.0f, -1.0f);
	MovementSpeed = SPEED_DEFAULT;
	MouseSensitivity = SENSITIVITY_DEFAULT;
	Zoom = ZOOM_DEFAULT;
	Near = NEAR_DEFAULT;
	Far = FAR_DEFAULT;
	Fov = FOV_DEFAULT;
	fov = FOV_DEFAULT;
	near = NEAR_DEFAULT;
	far = FAR_DEFAULT;
	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
	updateCameraVectors();
	return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
	return glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
	glm::mat4 view = getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;
	Yaw += xoffset;
	Pitch += yoffset;
	if (constrainPitch) {
		if (Pitch > 89.f) {
			Pitch = 89.f;
		}
		if (Pitch < -89.f) {
			Pitch = -89.f;
		}
	}
	updateCameraVectors();
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD) {
		Position += Front * velocity;
	}
	if (direction == BACKWARD) {
		Position -= Front * velocity;
	}
	if (direction == LEFT) {
		Position -= Right * velocity;
	}
	if (direction == RIGHT) {
		Position += Right * velocity;
	}
	if (direction == UP) {
		Position += velocity * Up;
	}
	if (direction == DOWN) {
		Position -= velocity * Up;
	}
}

void Camera::ProcessMouseScroll(float yoffset) {
	if (Zoom >= 1.f && Zoom <= MAX_FOV) {
		Zoom -= yoffset;
	}
	if (Zoom <= 1.0f) {
		Zoom = 1.0f;
	}
	if (Zoom >= MAX_FOV) {
		Zoom = MAX_FOV;
	}
	fov = Zoom;
}

void Camera::invertPitch() {
	this->Pitch = -Pitch;
	updateCameraVectors();
}

void Camera::updateCameraVectors() {
	if (std::isnan(Yaw) || std::isnan(Pitch)) {
		MyglobalLogger().logMessage(Logger::ERROR, "Failed the Yaw and Pitch.", __FILE__, __LINE__);  // Исправлено: убрали лишние *
		return;
	}
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

	if (std::isnan(front.x) || std::isnan(front.y) || std::isnan(front.z)) {
		MyglobalLogger().logMessage(Logger::ERROR, "Invalid Front vector calculated!", __FILE__, __LINE__);  // Исправлено: убрали лишние *
		return;
	}

	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}