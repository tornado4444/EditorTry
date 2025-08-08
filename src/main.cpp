#include "Window.hpp"
#include "Init.hpp"

auto main() -> int {
	Init init;

	init.initialize(); 

	while (!init.shouldClose()) {
		init.processInput(init.getWindow());
		init.render();
		init.swapBuffersAndPollEvents();
	}

	return 0;
}