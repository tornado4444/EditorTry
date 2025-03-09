#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// .hpp files
#include "../src/Logger/Logger.hpp"

static GLuint m_ProgramInUse = 0;

class Shader {
public:
	unsigned int ID = 0;	// create the ID where and it's for shaders(fragment and vertex shaders)
	unsigned int getId() const {	// getter for the ID
		return ID;
	}
public:
	Shader() : m_ID(0) {}											// constructor ñopy constructor
	Shader(const char* vertexPath, const char* fragmentPath);		// set parameters
	virtual ~Shader();												// just the destructor

public:
	virtual void use() const;
	virtual void setBool(const std::string& name, bool value) const;								// setter the bool 
	virtual void setInt(const std::string& name, int value) const;									// setter the int
	virtual void setFloat(const std::string& name, float value) const;								// setter the float
	virtual void setVec2(const std::string& name, glm::vec2 vector) const;							// setter the vec2
	virtual void setVec3(const std::string& name, glm::vec3 vector) const;							// setter the vec3
	virtual void setVec4(const std::string& name, glm::vec4 vector) const;							// setter the vec4
	virtual void setMat4(const std::string& name, glm::mat4 matrix) const;							// setter mat4
	virtual void setSampler2D(const std::string& name, unsigned int texture, int id) const;			// setter the sampler2D
	virtual void setSampler3D(const std::string& name, unsigned int texture, int id) const;			// setter the sampler 3D 

public:
	static void setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2);
	static void setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2, const GLfloat a_V3);
	static void setUniform(const char* a_Uniform, const GLfloat a_V0);
	//set shader uniform with 1 int
	static void setUniform(const char* a_Uniform, const GLuint a_V0);
	//set shader uniform with 1 int
	static void setUniform(const char* a_Uniform, const GLint a_V0);
	//set shader uniform with a matrix 4x4
	static void setUniform(const char* a_Uniform, const glm::mat4 a_V0);
	//set shader uniform with a vec2
	static void setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1);
	//set shader uniform with a vec2
	static void setUniform(const char* a_Uniform, const GLint a_V0, const GLint a_V1);

	static void setProjection(const glm::mat4 a_Projection);

protected:
	virtual void checkCompileErrors(unsigned int shader, std::string type, std::string shaderName);	// 
	virtual std::string getShaderName(const char* shaderPath);
	virtual std::string loadShaderFromFile(const char* shaderPath);
	virtual bool createFromString(const char* vertexSource, const char* fragmentSource);
private:
	unsigned m_ID;	
};
