#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "../src/Logger/Logger.hpp"

class Transform {
public:
	Transform();
	Transform(const Transform& Copy);
public:
	virtual void setDirty();
	virtual void reset();
	virtual void translate(glm::vec3 Translation, bool Word = true);
	virtual void scale(glm::vec3 Scale);
	virtual void vecRotate(glm::vec3 vecRotate);
	virtual void quatRotate(glm::quat quatRotate);

	virtual void rotateAround(glm::vec3 Rotation, glm::vec3 Pivot);
	virtual void oneAround(glm::vec3 Rotation);

	virtual void updateTransforms();
	virtual void setParent(Transform* hasParent);

public:
	glm::mat4 getGlobalMatrix();

	glm::vec3 getGlobalPosition();
	glm::vec3 getGlobalPosition(glm::vec3 Local);

	glm::vec3 getLocalPosition();
	glm::vec3 getLocalScale();
	glm::quat getLocalRotation();
	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();

private:
	bool hasDirty;
	glm::vec3 myLocalPosition;
	glm::quat myLocalRotation;
	glm::vec3 myLocalScale;
	glm::mat4 myLocalMatrix;
	glm::mat4 myGlobalMatrix;

private:
	Transform* Parent;
	std::vector<Transform*>Children;
};	