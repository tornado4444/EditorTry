#include "Transform.hpp"

Transform::Transform(){
	Parent = nullptr;
	reset();
	updateTransforms();
}

Transform::Transform(const Transform& a_Copy) {
	hasDirty = a_Copy.hasDirty;
	myLocalPosition = a_Copy.myLocalPosition;
	myLocalRotation = a_Copy.myLocalRotation;
	myLocalScale = a_Copy.myLocalScale;
	setParent(a_Copy.Parent);
}

void Transform::setDirty() {
	hasDirty = true;
}

void Transform::reset() {
	myLocalPosition = glm::vec3();
	myLocalScale = glm::vec3(1.0f);
	myLocalRotation = glm::quat();
	myLocalMatrix = myGlobalMatrix = glm::mat4();
	setDirty();
}

void Transform::translate(glm::vec3 Translation, bool Word) {
	if (Word) {
		myLocalPosition += Translation;
	}
	else {
		myLocalPosition += myLocalRotation * Translation;
	}
	setDirty();
}

void Transform::scale(glm::vec3 Scale) {
	myLocalScale = Scale;
	setDirty();
}

void Transform::vecRotate(glm::vec3 vecRotate) {
	myLocalRotation *= glm::quat(vecRotate);
	setDirty();
}

void Transform::quatRotate(glm::quat quatRotate) {
	myLocalRotation *= quatRotate;
	setDirty();
}

void Transform::rotateAround(glm::vec3 Rotation, glm::vec3 Pivot) {
	glm::vec3 dir = myLocalPosition - Pivot;
	dir = glm::quat(Rotation) * dir;
	myLocalPosition = dir + Pivot;
}

void Transform::oneAround(glm::vec3 Rotation) {
	glm::vec3 pivot = glm::vec3();

	if (Parent) {
		pivot = Parent->getLocalPosition();
	}
	glm::vec3 dir = myLocalPosition - pivot;
	dir = glm::quat(Rotation) * dir;
	myLocalPosition = dir;
}

glm::mat4 Transform::getGlobalMatrix() {
	updateTransforms();

	return myGlobalMatrix;
}

glm::vec3 Transform::getGlobalPosition() {
	glm::vec4 temp = glm::vec4(myLocalPosition.x, myLocalPosition.y, myLocalPosition.z, 1);

	temp = myGlobalMatrix * temp;

	glm::vec3 toVec3 = glm::vec3(temp.x, temp.y, temp.z);
	return  toVec3;
}

glm::vec3 Transform::getGlobalPosition(glm::vec3 Local) {
	glm::vec4 temp = glm::vec4(Local.x, Local.y, Local.z, 1);

	temp = myGlobalMatrix * temp;

	glm::vec3 toVec3 = glm::vec3(temp.x, temp.y, temp.z);
	return  toVec3;
}

glm::vec3 Transform::getLocalPosition() {
	return myLocalPosition;
}

glm::vec3 Transform::getLocalScale() {
	return myLocalScale;
}

glm::quat Transform::getLocalRotation() {
	return myLocalRotation;
}

glm::vec3 Transform::getForward() {
	updateTransforms();

	return glm::normalize(myLocalMatrix * glm::vec4(0, 0, 1, 0));
}

glm::vec3 Transform::getRight() {
	updateTransforms();

	return  glm::normalize(myLocalMatrix * glm::vec4(1, 0, 0, 0));
}

glm::vec3 Transform::getUp() {
	updateTransforms();

	return glm::normalize(myLocalMatrix * glm::vec4(0, 1, 0, 0));
}

void Transform::updateTransforms() {
	if (hasDirty) {
		myLocalMatrix = glm::mat4(1.0f);
		glm::mat4 rotationMatrix= toMat4(myLocalRotation);
		myLocalMatrix = glm::translate(myLocalMatrix, myLocalPosition);
		myLocalMatrix *= rotationMatrix;
		myLocalMatrix = glm::scale(myLocalMatrix, myLocalScale);
		hasDirty = false;

		for (int n = 0; n < Children.size(); n++) {
			Children[n]->updateTransforms();
		}
	}

	if (Parent == nullptr) {
		myGlobalMatrix = myLocalMatrix;
	}
	else {
		myGlobalMatrix = Parent->getGlobalMatrix() * myLocalMatrix;
	}
}

void Transform::setParent(Transform* hasParent) {
	Parent = hasParent;
	hasDirty = true;
}
