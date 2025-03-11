#include "Window.hpp"
#include "Engine.hpp"

auto main() -> int {
	Engine engine;

	engine.initialize();

	while (!engine.shouldClose()) {
		engine.processInput(engine.getWindow());
		engine.render();
		engine.swapBuffersAndPollEvents();
	}

	return 0;
}
