#pragma region --- INCLUDES ---
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#pragma endregion INCLUDES

//using namespace std;

#pragma region --- DEFINES ---
#pragma region --- QOL ---
#define print std::cout
#define printError std::cerr
#pragma endregion QOL

#pragma region --- FUN ---
#define yeet throw
#define broken_shoe std::runtime_error
#pragma endregion FUN
#pragma endregion DEFINES

#pragma region --- CONSTANTS ---
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const char* TITLE = "Emergine";
#pragma endregion CONSTANTS


class EmergineApp {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	#pragma region --- CLASS MEMBERS ---
	GLFWwindow* window;
	#pragma endregion CLASS MEMBERS

	#pragma region --- INIT WINDOW ---
	void initWindow() {
		// initialize glfw
		glfwInit();

		// tell glfw to not use OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// tell glfw to disable resizing windows (for now)
		// TODO: enable resizing windows
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
	}
	#pragma endregion INIT WINDOW

	#pragma region --- INIT VULKAN ---
	void initVulkan() {

	}
	#pragma endregion INIT VULKAN
	
	#pragma region --- MAIN LOOP ---
	void mainLoop() {
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}
	#pragma endregion MAIN LOOP
	
	#pragma region --- CLEANUP ---
	void cleanup() {
		// destroy the window
		glfwDestroyWindow(window);

		// terminate glfw
		glfwTerminate();
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
		printError << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
