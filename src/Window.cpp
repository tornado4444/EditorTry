#include "Window.hpp"

Window::Window() : window(nullptr) {
	if (!glfwInit()) {
		MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize GLFW.", __FILE__, __LINE__);
		return;
	}
	else {
		MyglobalLogger().logMessage(Logger::DEBUG, "GLFW initialized successfully.", __FILE__, __LINE__);
	}

	// Set OpenGL context version and profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// Create the window
	window = glfwCreateWindow(widthWindow, heightWindow, titleWindow(), nullptr, nullptr);
	if (!window || window == nullptr) {
		MyglobalLogger().logMessage(Logger::ERROR, "Failed to create GLFW window.", __FILE__, __LINE__);
		glfwTerminate();
		return;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize GLEW.", __FILE__, __LINE__);
		glfwTerminate();
		return;
	}

	// Set the framebuffer size callback
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Set initial viewport if window resize is enabled
	if (windowResize) {
		widthWindow = getWindowWidth();
		heightWindow = getWindowHeight();
		glViewport(0, 0, widthWindow, heightWindow);
		windowResize = true;
	}

	MyglobalLogger().logMessage(Logger::DEBUG, "Window initialization completed.", __FILE__, __LINE__);
}

bool Window::shouldClose() const {
	return glfwWindowShouldClose(window);
}

void Window::swapBuffersAndPollEvents() {
	// Swap front and back buffers
	glfwSwapBuffers(window);
	// Poll for and process events
	glfwPollEvents();
}

GLFWwindow* Window::getWindow() const {
	return window;
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	// Calculate offsets to maintain square aspect ratio
	int xOffset = 0;
	int yOffset = 0;
	int size = width < height ? width : height;

	// Center the square viewport
	xOffset = (width - size) / 2;
	yOffset = (height - size) / 2;

	// Set the viewport
	glViewport(xOffset, yOffset, size, size);

	// Set up orthographic projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

const char* Window::titleWindow() {
	return "OpenGL";
}

Window::~Window() {
	MyglobalLogger().logMessage(Logger::DEBUG, "Window is being destroyed.", __FILE__, __LINE__);
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}