#pragma once
#include <GL/glew.h>
#include "VBO.hpp"
#include "EBO.hpp"

class VAO {
public:
    VAO() {
        glGenVertexArrays(1, &ID);
    }

    ~VAO() {
        if (ID != 0) {
            glDeleteVertexArrays(1, &ID);
        }
    }

    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;

    VAO(VAO&& other) noexcept : ID(other.ID) {
        other.ID = 0;
    }

    VAO& operator=(VAO&& other) noexcept {
        if (this != &other) {
            if (ID != 0) {
                glDeleteVertexArrays(1, &ID);
            }
            ID = other.ID;
            other.ID = 0;
        }
        return *this;
    }

public:
    GLuint getID() const { return ID; }

    void Bind() const {
        glBindVertexArray(ID);
    }

    void UnBind() const {
        glBindVertexArray(0); 
    }

    void linkAttrib(const VBO& vbo, GLuint layout, GLint numComponents,
        GLenum type, GLsizei stride, const void* offset) const {
        vbo.Bind();
        glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(layout);
        vbo.UnBind();
    }

private:
    GLuint ID = 0; 
};