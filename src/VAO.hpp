#pragma once

#include <gl/glew.h>
#include "VBO.hpp"
#include "EBO.hpp"

class VAO {
public:
	VAO() {
		glGenVertexArrays(1, &ID);
	}

	~VAO() {
		glDeleteVertexArrays(1, &ID);
	}

public:
	GLuint getID() const { return ID; }
public:
	virtual void Bind() const {
		glBindVertexArray(ID);
	}

	virtual void UnBind() const {
		glBindVertexArray(ID);
	}

	virtual void linkAttrib(VBO& VBO, GLuint layout, GLint numComponents, GLenum type, GLsizei stride, const void* offset) const {
		glBindBuffer(GL_ARRAY_BUFFER, VBO.getID());
		glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
		glEnableVertexAttribArray(layout);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
private:
	GLuint ID;
};