// Camera.hpp
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.hpp"

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
public:
	static constexpr float YAW_DEFAULT = -90.0f;
	static constexpr float PITCH_DEFAULT = 0.0f;
	static constexpr float SPEED_DEFAULT = 10.0f;
	static constexpr float SENSITIVITY_DEFAULT = 0.1f;
	static constexpr float ZOOM_DEFAULT = 60.0f;
	static constexpr float FOV_DEFAULT = 45.0f;
	static constexpr float NEAR_DEFAULT = 0.1f;
	static constexpr float FAR_DEFAULT = 100.0f;
	static constexpr float MAX_FOV = 100.0f;

public:
	Camera() = default;
	Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
		float yaw = YAW_DEFAULT,
		float pitch = PITCH_DEFAULT) : Front(glm::vec3(0.f, 0.f, -1.f)),
		MovementSpeed(SPEED_DEFAULT), MouseSensitivity(SENSITIVITY_DEFAULT),
		Zoom(ZOOM_DEFAULT), Near(NEAR_DEFAULT), Far(FAR_DEFAULT),
		Fov(FOV_DEFAULT)
	{
		Position = position; WorldUp = up;
		Yaw = yaw; Pitch = pitch;
		fov = FOV_DEFAULT; near = NEAR_DEFAULT; far = FAR_DEFAULT;
		updateCameraVectors();
	}
	Camera(float pos_X, float pos_Y, float pos_Z, float up_X, float up_Y, float up_Z, float yaw, float pitch);

public:
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix(float aspectRatio) const;
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	void Matrix(Shader& shader, const char* uniform);

	virtual void ProcessMouseScroll(float yoffset);
	virtual void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	virtual void invertPitch();
	virtual void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

public:
	float fov, near, far;

public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	glm::vec3 getPosition() const { return Position; }

public:
	// GETTERS
	float getFov() const { return fov; }
	float getNear() const { return near; }
	float getFar() const { return far; }

public:
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	float Near;
	float Far;
	float Fov;
	float Yaw, Pitch;

private:
	virtual void updateCameraVectors();
};