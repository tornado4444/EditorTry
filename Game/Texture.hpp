#pragma once

// .hpp files
#include "../../src/Shader.hpp"
#include "../../src/Logger/Logger.hpp"

#include <GLFW/glfw3.h>
#include "stb_image.hpp"

class Texture {
public:
	Texture() = default;
	Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType);
	~Texture() = default;

public:
	GLuint ID; GLenum type; GLuint unit; 

public:
	inline virtual void texUnit(Shader& shader, const char* uniform, GLuint unit);
	inline virtual void bind();
	inline virtual void unbind();
	inline virtual void deleteShader();

public:
	unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma);
	unsigned int loadCubemap(std::vector<std::string> faces);
	unsigned int Texture2D(int w, int h);
	unsigned int Texture3D(int w, int h, int d);
};