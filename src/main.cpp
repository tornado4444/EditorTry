#include "Init.hpp"

auto main() -> int {
	Init init;

	init.initialize();

	while (!init.shouldClose()) {
		init.processInput(init.getWindow());
		init.render();	
		init.swapBuffersAndPollEvents();
	}
}
