#pragma once
#include <gl/glew.h>
#include <glfw/glfw3.h>
#include "../src/Logger/Logger.hpp"

class Window {
public:
	Window();
	~Window();

public:
	bool shouldClose() const;
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	virtual void swapBuffersAndPollEvents();

public:
	GLFWwindow* getWindow() const;
	virtual void setWindow(float width, float height) {
		widthWindow = width;
		heightWindow = height;
	}
	inline virtual float getWindowWidth() const { return widthWindow; }
	inline virtual float getWindowHeight() const { return heightWindow; }
	virtual const char* titleWindow();

protected:
	float widthWindow = 1920.0f;    
	float heightWindow = 1080.0f;
	bool windowResize = false;      

private:
	GLFWwindow* window;
};