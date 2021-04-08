#pragma region --- INCLUDES ---
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#pragma endregion INCLUDES

using namespace std;

#pragma region --- DEFINES ---
#pragma region --- QOL ---
#define print cout
#define printError cerr
#pragma endregion QOL

#pragma region --- FUN ---
#define yeet throw
#define broken_shoe runtime_error
#pragma endregion FUN

#pragma region --- VULKAN ---
// debug severities
#define VK_DEBUG_VERBOSE VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
#define VK_DEBUG_INFO VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
#define VK_DEBUG_WARNING VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
#define VK_DEBUG_ERROR VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT

// debug types
#define VK_DEBUG_TYPE_GENERAL VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
#define VK_DEBUG_TYPE_VALIDATION VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
#define VK_DEBUG_TYPE_PERFORMANCE VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
#pragma endregion VULKAN

#pragma endregion DEFINES

#pragma region --- CONSTANTS ---
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const char* TITLE = "Emergine";
const auto VERSION = VK_MAKE_VERSION(0, 1, 2);

const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG

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
	// the window holding the application
	GLFWwindow* window;

	// the vulkan instance
	VkInstance instance;

	// vulkan debug messenger
	VkDebugUtilsMessengerEXT debugMessenger;
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
		createInstance();
		setupDebugMessenger();
	}

	#pragma region --- CREATE INSTANCE ---
	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			yeet broken_shoe("validation layers requested, but not available!");
		}

		#pragma region --- APP INFO ---
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = TITLE;
		appInfo.applicationVersion = VERSION;
		appInfo.pEngineName = "Emergine";
		appInfo.engineVersion = VERSION;
		// minimum api version
		appInfo.apiVersion = VK_API_VERSION_1_0;
		#pragma endregion APP INFO
		
		#pragma region --- CREATE INFO ---
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		#pragma region --- GLOBAL EXTENSIONS ---
		auto globalExtensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(globalExtensions.size());
		createInfo.ppEnabledExtensionNames = globalExtensions.data();
		#pragma endregion GLOBAL EXTENSIONS

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
		
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			yeet broken_shoe("failed to create instance!");
		}
		#pragma endregion CREATE INFO

		#pragma region --- EXTENSION SUPPORT ---
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		// print supported extensions
		print << "available extensions:\n";

		for (const auto& extension : extensions)
		{
			print << '\t' << extension.extensionName << '\n';
		}
		#pragma endregion EXTENSION SUPPORT
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProps : availableLayers) {
				if (strcmp(layerName, layerProps.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
	#pragma endregion CREATE INSTANCE

	#pragma region --- DEBUG ---
	void setupDebugMessenger() {
		if (!enableValidationLayers)
		{
			return;
		}

		#pragma region --- CREATE INFO ---
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		// default severities = VK_DEBUG_VERBOSE | VK_DEBUG_WARNING | VK_DEBUG_ERROR;
		createInfo.messageSeverity = VK_DEBUG_VERBOSE| VK_DEBUG_WARNING | VK_DEBUG_ERROR;
		// default types = VK_DEBUG_TYPE_GENERAL | VK_DEBUG_TYPE_VALIDATION | VK_DEBUG_TYPE_PERFORMANCE;
		createInfo.messageType = VK_DEBUG_TYPE_GENERAL | VK_DEBUG_TYPE_VALIDATION | VK_DEBUG_TYPE_PERFORMANCE;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
		#pragma endregion

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to set up debug messenger!");
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
		VkDebugUtilsMessageTypeFlagsEXT msgType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		print << "validation layer: " << pCallbackData->pMessage << endl;

		return VK_FALSE;
	}

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		VkAllocationCallbacks* pAllocator, // heh, Pal-Locator
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	#pragma endregion DEBUG
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
		// destroy the debugger
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// destroy the vulkan instance
		vkDestroyInstance(instance, nullptr);

		// destroy the window
		glfwDestroyWindow(window);

		// terminate glfw
		glfwTerminate();
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
	#pragma endregion CLEANUP
};

int main() {
	EmergineApp app;

	try
	{
		app.run();
	}
	catch (const exception& e)
	{
		printError << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
