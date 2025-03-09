#include "Window.hpp"

// the consructor of window
Window::Window() : window(nullptr) {
	if (!glfwInit()) {		// Initializes the GLFW library
		MyglobalLogger().logMessage(Logger::ERROR, "Failed to create the window.", __FILE__, __LINE__);
		return;
	}
	else {
		MyglobalLogger().logMessage(Logger::DEBUG, "Nice to create window.", __FILE__, __LINE__);
	}
	window = glfwCreateWindow(widthWindow, heightWindow, titleWindow(), nullptr, nullptr);	// just create the window
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	if (!window || window == nullptr) {		// to check if the nothing the window or equal the zero
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;		// we're to we indicate that the window will always be true
	if (glewInit() != GLEW_OK || glewInit == GL_FALSE) {
		MyglobalLogger().logMessage(Logger::ERROR, "Nice to create window.", __FILE__, __LINE__);
		glfwTerminate();	// this call will close all open windows and release allocated resources
		return;
	}
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (windowResize) {
		widthWindow = getWindowWidth();
		heightWindow = getWindowHeight();
		glViewport(0, 0, widthWindow, heightWindow);

		windowResize = true;
	}
	MyglobalLogger().logMessage(Logger::DEBUG, "Initialize starting...", __FILE__, __LINE__);
}

bool Window::shouldClose() const {		// it's for the close window
	return glfwWindowShouldClose(window);	// close the window
}

void Window::swapBuffersAndPollEvents() {
	glfwSwapBuffers(window); // replaces the color buffer (a large buffer containing color values ​​for each pixel in the GLFW window) that was used for rendering during the current iteration and displays the result on the screen.
	glfwPollEvents(); // monitors whether any events are fired
}

// getter the window
GLFWwindow* Window::getWindow() const {
	return window;
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	int xOffset = 0;							 // i set the x coordinate axis (default 0)
	int yOffset = 0;							 // i set the y coordinate axis (default 0)
	int size = width < height ? width : height;  // Take the smaller side

	// Centering a square viewport
	xOffset = (width - size) / 2;				// to the axis x the width
	yOffset = (height - size) / 2;				// to the axis y the height

	glViewport(xOffset, yOffset, size, size);

	// Set up the projection
	glMatrixMode(GL_PROJECTION);				// sets the current matrix mode(in our case it is projection)
	glLoadIdentity();							// replaces the current matrix with the identity matrix.
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);	// describes a perspective matrix that creates a parallel projection

	glMatrixMode(GL_MODELVIEW);					// sets the current matrix mode(in our case it is modelview)
	glLoadIdentity();							// replaces the current matrix with the identity matrix.
}

// the title our window
const char* Window::titleWindow() {
	return "OpenGL";
}

Window::~Window() {
	MyglobalLogger().logMessage(Logger::DEBUG, "Window is destroying.", __FILE__, __LINE__);
	glfwDestroyWindow(window);	// the destroy window if we can
	glfwTerminate();			// this call will close all open windows and release allocated resources
}