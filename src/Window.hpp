#pragma once

#include <gl/glew.h>
#include <glfw/glfw3.h>

// .hpp files on here
#include "../src/Logger/Logger.hpp"
#include "../../Game/Camera.hpp"


class Window {
public:
	Window();	// the default constructor
	~Window();	// destructor

public:
	bool shouldClose() const; // the function on the close the Window

	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);	// for the framebufferCallBack
	virtual void swapBuffersAndPollEvents();										// just for swap buffers

public:
	GLFWwindow* getWindow() const;		// the getter
	// setter the window
	virtual void setWindow(float width, float height) {
		widthWindow = width;
		heightWindow = height;
	}

	inline virtual float getWindowWidth() const { return widthWindow; }		// getter for the return width
	inline virtual float getWindowHeight() const { return heightWindow; }	// getter for the return height

	// the title
	virtual const char* titleWindow(); // the title of the window
protected:
	float widthWindow  = 1980.0f;	 // width the window
	float heightWindow = 1080.0f;    // height the window

	bool windowResize;			 // options for changing the window
private:
	GLFWwindow* window;		// announce a window
	Camera* camera = nullptr;

private:
	int ñhangeWidth = widthWindow; int heightHeight = heightWindow;
};