#include "Shader.hpp"

// Shader class constructor: loads, compiles, and links shaders.

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vShaderCode = loadShaderFromFile(vertexPath); // to load the vertex shader
	std::string fShaderCode = loadShaderFromFile(fragmentPath); // to load the pixel shader

	const char* vShaderString = vShaderCode.c_str(); // initialize the vertex shader itself
	const char* fShaderString = fShaderCode.c_str(); // initialize the pixel shader itself

	// compile shaders
	unsigned int vertex, fragment;      // shader identifiers	    

	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER); // initialize the vertex shader
	glShaderSource(vertex, 1, &vShaderString, NULL); // set the shader source code
	glCompileShader(vertex); // compile the shader
	checkCompileErrors(vertex, "VERTEX", getShaderName(vertexPath)); // check for compilation errors

	// pixel shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER); // create a shader object of fragment type
	glShaderSource(fragment, 1, &fShaderString, NULL); // set the shader source code
	glCompileShader(fragment); // compile the shader
	checkCompileErrors(fragment, "FRAGMENT", getShaderName(fragmentPath)); // check for compilation errors

	// creating and linking a shader program
	ID = glCreateProgram(); // create an OpenGL program object
	glAttachShader(ID, vertex); // attach the vertex shader to the program
	glAttachShader(ID, fragment); // attach the fragment shader to the program
	glLinkProgram(ID); // link the shader program
	checkCompileErrors(ID, "PROGRAM", ""); // check shader compilation

	// remove shaders, since they are now associated with our program and are no longer needed
	glDeleteShader(vertex); // remove the vertex shader
	glDeleteShader(fragment); // remove the pixel shader
	// Log of successful download
	MyglobalLogger().logMessage(Logger::DEBUG,
		"SHADERS " + getShaderName(vertexPath) + " AND " + getShaderName(fragmentPath) + " LOADED AND COMPILED!", __FILE__, __LINE__
	);
}

// THE DESTRUCTOR
Shader::~Shader() {
	if (ID != 0) {
		glDeleteProgram(ID);
		ID = 0;		// protect against reuse
	}
	MyglobalLogger().logMessage(Logger::DEBUG, "Shader program deleted.", __FILE__, __LINE__);
}

void Shader::use() const { // Activates (uses) the current shader program
	glUseProgram(ID); // sets the program object specified by the value of the program parameter as part of the current rendering state.
}

// Sets a uniform variable of type bool
void Shader::setBool(const std::string& name, bool value) const { // sets a uniform variable of type bool
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); // assign a location value to the texture sampler to install several textures at once in the fragment shader.
}

// Sets a uniform variable of type int
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);	// used to change the value of the uniform variable specified by location
}

// Sets a float uniform variable
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);	// used to change the value of the uniform variable specified by location(1 - for float, int, unsigned, int, bool)
}

// Set a uniform variable type vec2
void Shader::setVec2(const std::string& name, glm::vec2 vector) const
{
	unsigned int location = glGetUniformLocation(ID, name.c_str());	// the location(returns an integer that represents the location of a specific uniform variable within a program object) 
	glUniform2fv(location, 1, glm::value_ptr(vector));				// used to change the value of the uniform variable specified by location(2 - for vec2, ivec2, uvec2, bvec2, etc)
}

// Set uniform variable type vec3
void Shader::setVec3(const std::string& name, glm::vec3 vector) const
{
	unsigned int location = glGetUniformLocation(ID, name.c_str());	// the location(returns an integer that represents the location of a specific uniform variable within a program object) 
	glUniform3fv(location, 1, glm::value_ptr(vector));				// can be used to load a uniform variable array of type vec3
}

//  Set uniform variable type vec4
void Shader::setVec4(const std::string& name, glm::vec4 vector) const
{
	unsigned int location = glGetUniformLocation(ID, name.c_str());	 // the location(returns an integer that represents the location of a specific uniform variable within a program object) 
	glUniform4fv(location, 1, glm::value_ptr(vector));				 //  can be used to load a uniform variable array of type vec4	
}

// Set uniform variable type mat4 4x4
void Shader::setMat4(const std::string& name, glm::mat4 matrix) const
{
	unsigned int mat = glGetUniformLocation(ID, name.c_str());	// the mat(returns an integer that represents the location of a specific uniform variable within a program object)
	// if our mat mat meaning
	if (mat == -1) {
		// for the just logger output
		MyglobalLogger().logMessage(Logger::ERROR,
			"Failed to find uniform: " + name, __FILE__, __LINE__);
		return;
	}
	glUniformMatrix4fv(mat, 1, false, glm::value_ptr(matrix));		// can be used to load a uniform variable array of type vec4
}

// Set the texture 2D in uniform
void Shader::setSampler2D(const std::string& name, unsigned int texture, int id) const
{
	glActiveTexture(GL_TEXTURE0 + id);		// active the texture
	glBindTexture(GL_TEXTURE_2D, texture);
	this->setInt(name, id);					// to send in setInt
}

// Setting the 3D texture to a uniform variable
void Shader::setSampler3D(const std::string& name, unsigned int texture, int id) const
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_3D, texture);
	this->setInt(name, id);
}

// Check on the errors
void Shader::checkCompileErrors(unsigned int shader, std::string type, std::string shaderName)
{
	int success;				// variable for error checking
	char infoLog[1024];			// buffer for error messages[1024]
	if (type != "PROGRAM")		// if the type isn't program
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // check the status to compile shader
		if (!success) // if not correct compilation
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog); // Get compilation error log
			std::ostringstream logMessage;					 // create a thread to generate a message string
			logMessage << "ERROR: SHADER " << shaderName
				<< " COMPILATION ERROR of type: " << type
				<< "\n" << infoLog
				<< "\n-------------------------------------------------------"; // cout the information
			MyglobalLogger().logMessage(Logger::ERROR, logMessage.str(), __FILE__, __LINE__); // logger
		}
	}
	else // if the shader type is "PROGRAM" (that is, shader program)
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success); // check the linking status of the program
		if (!success) // if linking not correct
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog); // display a number of errors
			std::ostringstream logMessage;					  // create a thread to generate a message string
			logMessage << "ERROR::PROGRAM_LINKING_ERROR " << shaderName
				<< " of type: " << type
				<< "\n" << infoLog
				<< "\n-------------------------------------------------------"; // cout the infomation
			MyglobalLogger().logMessage(Logger::ERROR, logMessage.str(), __FILE__, __LINE__); // logger
		}
	}
}

// Загружает содержимое шейдерного файла в строку
std::string Shader::loadShaderFromFile(const char* shaderPath) {
	std::string shaderCode; // переменная для хранения содержимого шейдерного файла
	std::ifstream shaderFile; // поток для чтения из файла
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit); // устанавливаем исключения для обработки ошибок ввода/вывода
	try
	{
		// открываю файлы в указаном пути
		shaderFile.open(shaderPath);
		std::stringstream shaderStream; // поток для накопления данных из файла
		shaderStream << shaderFile.rdbuf(); // читаем содержимое файла в поток
		shaderFile.close(); // закрываю файл
		shaderCode = shaderStream.str(); // конвертируем содержимое потока в строку
	}
	catch (std::ifstream::failure e) // обрабатываю исключение при обработки ошибки
	{
		// Обрабатываю вывод
		MyglobalLogger().logMessage(Logger::ERROR,
			"ERROR::SHADER " + getShaderName(shaderPath) + "FILE_NOT_SUCCESFULLY_READ!", __FILE__, __LINE__
		);
	}
	return shaderCode; // возращаю содержимое файла
}

std::string Shader::getShaderName(const char* path) {
	std::string pathstr = std::string(path); // преобразуем путь в строку std::string
	const size_t last_slash_idx = pathstr.find_last_of("/"); // находим последний символ '/' 
	if (std::string::npos != last_slash_idx) // если '/' найден
	{
		pathstr.erase(0, last_slash_idx + 1); // удаляем всё до и включая последний '/'
	}
	return pathstr; // возвращаю оставшуюся часть строки — имя файла
}

bool Shader::createFromString(const char* vertexSource, const char* fragmentSource) {
	// Создание вершинного шейдера
	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX", "string");

	// Создание фрагментного шейдера
	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT", "string");

	// Создание шейдерной программы
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM", "string");

	// Удаление шейдеров, они уже связаны с программой
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return true;
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2) {
	glUniform3f(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0, a_V1, a_V2);
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2, const GLfloat a_V3) {
	glUniform4f(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0, a_V1, a_V2, a_V3);
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0) {
	glUniform1f(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0);
}

void Shader::setUniform(const char* a_Uniform, const GLuint a_V0) {
	glUniform1i(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0);
}

void Shader::setUniform(const char* a_Uniform, const GLint a_V0) {
	glUniform1i(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0);
}

void Shader::setUniform(const char* a_Uniform, const glm::mat4 a_V0) {
	glUniformMatrix4fv(glGetUniformLocation(m_ProgramInUse, a_Uniform), 1, GL_FALSE, glm::value_ptr(a_V0));
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1) {
	glUniform2f(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0, a_V1);
}

void Shader::setUniform(const char* a_Uniform, const GLint a_V0, const GLint a_V1) {
	glUniform2i(glGetUniformLocation(m_ProgramInUse, a_Uniform), a_V0, a_V1);
}

void Shader::setProjection(const glm::mat4 a_Projection) {
	glUniformMatrix4fv(glGetUniformLocation(m_ProgramInUse, "projection"), 1, GL_FALSE, glm::value_ptr(a_Projection));
}

