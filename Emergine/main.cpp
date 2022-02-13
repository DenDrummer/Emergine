#pragma region --- INCLUDES ---
// window library with built in vulkan include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std imports
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
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

// device properties & features
#define VK_TYPE_DISCRETE_GPU VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
#pragma endregion VULKAN
#pragma endregion DEFINES

#pragma region --- CONSTANTS ---
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const char* TITLE = "Emergine";
const auto VERSION = VK_MAKE_VERSION(0, 1, 3);

const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG

#pragma endregion CONSTANTS

#pragma region --- STRUCTS ---
struct QueueFamilyIndices
{
	optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};
#pragma endregion STRUCTS



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

	// physical device (aka GPU)
	// implicitly destroyed with the vulkan instance
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
		pickPhysicalDevice();
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
		// see VkVersions.md
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

		#pragma region --- VALIDATION LAYERS ---
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
		#pragma endregion VALIDATION LAYERS
		
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			yeet broken_shoe("failed to create instance!");
		}
		#pragma endregion CREATE INFO

		printExtensionSupport();
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

	void printExtensionSupport() {
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
	}
	#pragma endregion CREATE INSTANCE

	#pragma region --- PHYSICAL DEVICE ---
	void pickPhysicalDevice() {
		// count devices with vulkan support
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			yeet broken_shoe("failed to find GPUs with Vulkan support!");
		}

		print << "devices with vulkan support: " << deviceCount << endl;

		// store all compatible devices in a vector
		vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		#pragma region --- SUITABILITY CHECKs ---
		multimap<VkPhysicalDevice, int> candidates;
		for (const VkPhysicalDevice& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				int score = rateDeviceSuitability(device);
				candidates.insert(make_pair(device, score));
				// for debugging purposes
				VkPhysicalDeviceProperties props;
				vkGetPhysicalDeviceProperties(device, &props);
				print << '\t' << props.deviceName << '\t' << score << endl;
			}
		}

		// check if the best candicate is suitable at all
		if (candidates.rbegin()->first > 0)
		{
			physicalDevice = candidates.rbegin()->first;
		}
		else
		{
			yeet broken_shoe("failed to find a suitable GPU!");
		}
		#pragma endregion SUITABILITY CHECKs
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		#pragma region --- EXAMPLE CHECKS ---
		/*// get device properties
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);

		// get device features
		VkPhysicalDeviceFeatures feats;
		vkGetPhysicalDeviceFeatures(device, &feats);

		// check if dedicated graphics card which supports:
		// • geometry shaders
		return props.deviceType == VK_TYPE_DISCRETE_GPU
			&& feats.geometryShader;*/
		#pragma endregion EXAMPLE CHECKS

		QueueFamilyIndices indices = findQueueFamilies(device);

		return indices.isComplete();
	}

	int rateDeviceSuitability(VkPhysicalDevice device) {
		#pragma region --- EXAMPLE SCORE ---
		/*VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);

		// get device features
		VkPhysicalDeviceFeatures feats;
		vkGetPhysicalDeviceFeatures(device, &feats);

		int score = 0;

		// Discrete GPUs have a significan performance advantage
		if (props.deviceType == VK_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += props.limits.maxImageDimension2D;

		// Application can't function without geometry shaders
		if (!feats.geometryShader)
		{
			return 0;
		}

		return score;*/
		#pragma endregion EXAMPLE SCORE

		return 1;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}
	#pragma endregion PHYSICAL DEVICE
	#pragma endregion INIT VULKAN

	#pragma region --- DEBUG ---
	void setupDebugMessenger() {
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to set up debug messenger!");
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		// default severities = VK_DEBUG_VERBOSE | VK_DEBUG_WARNING | VK_DEBUG_ERROR;
		createInfo.messageSeverity = VK_DEBUG_VERBOSE | VK_DEBUG_WARNING | VK_DEBUG_ERROR;
		// default types = VK_DEBUG_TYPE_GENERAL | VK_DEBUG_TYPE_VALIDATION | VK_DEBUG_TYPE_PERFORMANCE;
		createInfo.messageType = VK_DEBUG_TYPE_GENERAL | VK_DEBUG_TYPE_VALIDATION | VK_DEBUG_TYPE_PERFORMANCE;
		createInfo.pfnUserCallback = debugCallback;
		// optional
		createInfo.pUserData = nullptr;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
		VkDebugUtilsMessageTypeFlagsEXT msgType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		printError << "validation layer: " << pCallbackData->pMessage << endl;

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

	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
	#pragma endregion DEBUG
	
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
