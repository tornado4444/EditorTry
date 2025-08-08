#include "Window.hpp"

Window::Window() : window(nullptr) {
	if (!glfwInit()) {		
		MyglobalLogger().logMessage(Logger::ERROR, "Failed to create the window.", __FILE__, __LINE__);
		return;
	}
	else {
		MyglobalLogger().logMessage(Logger::DEBUG, "Nice to create window.", __FILE__, __LINE__);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	window = glfwCreateWindow(widthWindow, heightWindow, titleWindow(), nullptr, nullptr);	
	if (!window || window == nullptr) {		
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;		
	if (glewInit() != GLEW_OK || glewInit == GL_FALSE) {
		MyglobalLogger().logMessage(Logger::ERROR, "Nice to create window.", __FILE__, __LINE__);
		glfwTerminate();	
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

bool Window::shouldClose() const {		
	return glfwWindowShouldClose(window);	
}

void Window::swapBuffersAndPollEvents() {
	glfwSwapBuffers(window); 
	glfwPollEvents(); 
}

GLFWwindow* Window::getWindow() const {
	return window;
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	int xOffset = 0;							
	int yOffset = 0;							
	int size = width < height ? width : height; 
	xOffset = (width - size) / 2;				
	yOffset = (height - size) / 2;				

	glViewport(xOffset, yOffset, size, size);

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
	MyglobalLogger().logMessage(Logger::DEBUG, "Window is destroying.", __FILE__, __LINE__);
	glfwDestroyWindow(window);	
	glfwTerminate();			
}