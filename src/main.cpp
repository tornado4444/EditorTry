#include "Window.hpp"
#include "Engine.hpp"

int main() {
	Engine engine;

	engine.initialize();

	while (!engine.shouldClose()) {
		engine.processInput(engine.getWindow());
		engine.render();
		engine.swapBuffersAndPollEvents();
	}

	return 0;
}