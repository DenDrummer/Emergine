#include <vulkan/vulkan.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

class EmergineApp {
public:
	void run() {
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	#pragma region --- INIT VULKAN ---
	void initVulkan() {

	}
	#pragma endregion INIT VULKAN
	
	#pragma region --- MAIN LOOP ---
	void mainLoop() {

	}
	#pragma endregion MAIN LOOP
	
	#pragma region --- CLEANUP ---
	void cleanup() {

	}
	#pragma endregion CLEANUP
};

int main() {
	EmergineApp app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
