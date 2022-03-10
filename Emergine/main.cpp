#pragma region --- INCLUDES ---
// window library with built in vulkan include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std imports
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
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
const auto VERSION = VK_MAKE_VERSION(0, 1, 12);

#pragma region --- VALIDATION LAYERS ---
const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG
#pragma endregion VALIDATION LAYERS

const vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const string VERT_SHADER_PATH = "shaders/vert.spv";
const string FRAG_SHADER_PATH = "shaders/frag.spv";
#pragma endregion CONSTANTS

#pragma region --- STRUCTS ---
struct QueueFamilyIndices
{
	optional<uint32_t> graphicsFamily;
	optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value()
			&& presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;
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
	// window surface (aka the canvas of the window on which things get drawn)
	VkSurfaceKHR surface;
	
	#pragma region --- DEVICES ---
	// physical device (aka GPU)
	// implicitly destroyed with the vulkan instance
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	// logical device
	VkDevice device;
	#pragma endregion DEVICES
	
	#pragma region --- QUEUES ---
	VkQueue graphicsQueue;
	// presentation queue
	VkQueue presentQueue;
	#pragma endregion QUEUES

	#pragma region --- SWAP CHAIN ---
	VkSwapchainKHR swapChain;
	vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	#pragma endregion SWAP CHAIN

	vector<VkImageView> swapChainImageViews;

	VkPipelineLayout pipelineLayout;
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
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
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

	#pragma region --- CREATE SURFACE ---
	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to create window surface!");
		}
	}
	#pragma endregion CREATE SURFACE

	#pragma region --- SELECT PHYSICAL DEVICE ---
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

		// TODO? store indices as class member?
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete()
			&& extensionsSupported // redundant? as swapChainAdequate will always be false if this is false
			&& swapChainAdequate;
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

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const VkExtensionProperties& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		// surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		#pragma region --- SURFACE FORMATS ---
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		#pragma endregion SURFACE FORMATS

		#pragma region --- PRESENTATION MODES ---
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		#pragma endregion PRESENTATION MODES

		return details;
	}
	#pragma endregion SELECT PHYSICAL DEVICE

	#pragma region --- CREATE LOGICAL DEVICE ---
	void createLogicalDevice() {
		// TODO? store indices as class member?
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		#pragma region --- QUEUE CREATE INFO ---
		vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			// create new queue
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			// add new queue to list of queues
			queueCreateInfos.push_back(queueCreateInfo);
		}

		#pragma endregion QUEUE CREATE INFO

		VkPhysicalDeviceFeatures deviceFeatures{};
		
		#pragma region --- DEVICE CREATE INFO ---
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<int32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// ignored by up-to-date implementations,
		// but assigned for more backwards compatibility
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
		#pragma endregion DEVICE CREATE INFO

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
	#pragma endregion CREATE LOGICAL DEVICE

	#pragma region --- CREATE SWAP CHAIN ---
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// using minimum may cause waiting times,
		// due to having to wait on new free image to start rendering to
		// TODO? swap "swapChainSupport.capabilities" for "extent"
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		// no maximum limit --> max = 0
		// minimum might be equal to maximum --> put imageCount back to min
		// TODO? swap "swapChainSupport.capabilities" for "extent"
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		#pragma region --- SWAP CHAIN CREATE INFO ---
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		
		#pragma region --- IMAGE DETAILS ---
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;

		// always 1 unless making stereoscopic 3D
		createInfo.imageArrayLayers = 1;

		// specifies the kind of operations the images will be used for
		// here: render directly to them --> VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		// render to other image first (e.g. for post-processing) --> something like VK_IMAGE_USAGE_TRANSFER_DST_BIT
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		#pragma endregion IMAGE DETAILS

		#pragma region --- QUEUE FAMILIES ---
		// TODO? store indices as class member?
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		if (indices.graphicsFamily != indices.presentFamily)
		{
			// imgs can be used accross multiple queue families without explicit ownership transfers
			// requires at least 2 distinct queue families
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			// img owned by 1 queue family at a time and ownership must be explicitly transferred before using in another
			// best performance
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // optional
			createInfo.pQueueFamilyIndices = nullptr; // optional
		}
		#pragma endregion QUEUE FAMILIES

		// can specify transform to images if transform listed in capabilities.supportedTransforms
		// no transform --> currentTransform
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// almost always ignore alpha channel --> VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		// VK_TRUE --> don't care about color of obscured pixels, but higher performance
		createInfo.clipped = VK_TRUE;

		// TODO: needed for window resizing
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		#pragma endregion SWAP CHAIN CREATE INFO

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// you could rate formats,
		// but usually first specified format is okay as backup if there's no SRGB
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes) {
		// see PresentationModes.md for explanation
		for (const VkPresentModeKHR& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		// if images can be different resolution than window,
		// width and height in currentExtent will be set to UINT32_MAX
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else // TODO: redundant else due to always returning in if?
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	#pragma endregion CREATE SWAP CHAIN
	
	#pragma region --- CREATE IMAGE VIEWS ---
	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		// create an image view for each of the images on the queue
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			#pragma region --- CREATE INFO ---
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];

			// viewType and format specify how the image data should be interpreted
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;

			// components allow you to swizzle (shuffle) the color channelz around
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			// subresourceRange describes image's purpose and which part should be accessed
			// here: color targets without mipmapping and only 1 layer
			// if stereographic 3D: swapchain with multiple layers;
			//		multiple image views for each image, representing views for left and right eyes by accessing different layers
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			#pragma endregion CREATE INFO

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				yeet broken_shoe("failed to create image views!");
			}
		}
	}
	#pragma endregion CREATE IMAGE VIEWS

	#pragma region --- CREATE RENDER PASSES ---
	void createRenderPass() {
		#pragma region --- COLOR ATTACHMENT ---
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		/// determines what to do with data in attachment before rendering
		// main options:
		//		VK_ATTACHMENT_LOAD_OP_LOAD: Preserve existing contents
		//		VK_ATTACHMENT_LOAD_OP_CLEAR: Clear values to a constant
		//		VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are set to undefined
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		/// determines what to do with data in attachment after rendering
		// main options:
		//		VK_ATTACHMENT_STORE_OP_STORE: Rendered contents stored in memory and can be read later
		//		VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of framebuffer will be undefined
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// most common image layouts:
		//		VK_IMAGE_LAYOUT_UNDEFINED: don't care, but contents of image not guaranteed to be preserved
		//		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
		//		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
		//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		#pragma endregion COLOR ATTACHMENT
	}
	#pragma endregion CREATE RENDER PASSES

	#pragma region --- CREATE GRAPHICS PIPELINE ---
	void createGraphicsPipeline() {
		#pragma region --- SHADER STAGES ---
		// readFile returns vector<char>
		auto vertShaderCode = readFile(VERT_SHADER_PATH);
		auto fragShaderCode = readFile(FRAG_SHADER_PATH);

		// shader modules only required fore pipeline creation, so only required locally
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		#pragma region --- VERT SHADER STAGE CREATE INFO ---
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		// optional, used to specify contstants defined at pipeline creation
		vertShaderStageInfo.pSpecializationInfo = nullptr;
		#pragma endregion VERT SHADER STAGE CREATE INFO

		#pragma region --- FRAG SHADER STAGE CREATE INFO ---
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		// optional, used to specify contstants defined at pipeline creation
		fragShaderStageInfo.pSpecializationInfo = nullptr;
		#pragma endregion FRAG SHADER STAGE CREATE INFO

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
		#pragma endregion SHADER STAGES
		
		#pragma region --- FIXED FUNCTIONS ---
		#pragma region --- VERTEX INPUT ---
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		#pragma endregion VERTEX INPUT
		
		#pragma region --- INPUT ASSEMBLY ---
		// example topologies:
		//		VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
		//		VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
		//		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: end vertex of every line is used as start vertex for next line
		//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
		//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
		// with an element buffer, indices can be manually specified, allowing optimizations like vertex reuse
		// if primitiveRestartEnable set to VK_TRUE, possible to break up lines and triangles in _STRIP topologies by using 0xFFFF or 0xFFFFFFFF index
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		#pragma endregion INPUT ASSEMBLY
		
		#pragma region --- VIEWPORT STATE ---
		#pragma region --- VIEWPORT ---
		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		#pragma endregion VIEWPORT

		#pragma region --- SCISSOR ---
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		#pragma endregion SCISSOR

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		// TODO: possibly required to change to 2 viewports (and scissors?) for VR? would require GPU feature
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		#pragma endregion VIEWPORT STATE
		
		#pragma region --- RASTERIZER ---
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// depthClampEnable = VK_FALSE : fragments beyond the depth clamp are discarded
		// depthClampEnable = VK_TRUE : fragments beyond the depth clamp are clamped, requires a GPU-feature.
		//		useful in special cases like shadowmaps
		rasterizer.depthClampEnable = VK_FALSE;

		// if rasterizerDiscardEnable is set to VK_TRUE, geometry never passes through the rasterizer.
		// basically disables output to the framebuffer
		rasterizer.rasterizerDiscardEnable = VK_FALSE;

		// polygonMode determines how fragments are generated for geometry.
		// main modes:
		//		VK_POLYGON_MODE_FILL	: fill the area of the polygon with fragments
		//		VK_POLYGON_MODE_LINE	: polygon edges are drawn as lines
		//		VK_POLYGON_MODE_POINT	: polygon vertices are drawn as points
		// modes other than _FILL require a GPU-feature
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

		// max width depends on hardware
		// any line thicker than 1.0f requires the wideLines GPU-feature
		rasterizer.lineWidth = 1.0f;

		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		// rasterizer can alter depth values.
		// sometimes used for shadow mapping
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // optional
		rasterizer.depthBiasClamp = 0.0f; // optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // optional
		#pragma endregion RASTERIZER
		
		#pragma region --- MULTISAMPLING ---
		// enabling this requires a GPU feature
		// will be revisited
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;			// optional
		multisampling.pSampleMask = nullptr;			// optional
		multisampling.alphaToCoverageEnable = VK_FALSE;	// optional
		multisampling.alphaToOneEnable = VK_FALSE;		// optional
		#pragma endregion MULTISAMPLING
		
		#pragma region --- DEPTH AND STENCIL TESTING ---
		// required when using depth and/or stencil buffer
		// will be revisited
		// uses VkPipelineDepthStencilStateCreateInfo
		#pragma endregion DEPTH AND STENCIL TESTING
		
		#pragma region --- COLOR BLENDING ---
		#pragma region --- COLOR BLEND ATTACHMENT ---
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;		// optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;				// optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		// optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;				// optional
		#pragma endregion COLOR BLEND ATTACHMENT
		
		#pragma region --- COLOR BLEND STATE ---
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		// set logicOpEnable to VK_TRUE for bitwise blending, will disable regular blending
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;	// optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;		// optional
		colorBlending.blendConstants[1] = 0.0f;		// optional
		colorBlending.blendConstants[2] = 0.0f;		// optional
		colorBlending.blendConstants[3] = 0.0f;		// optional
		#pragma endregion COLOR BLEND STATE
		#pragma endregion COLOR BLENDING
		
		#pragma region --- DYNAMIC STATE ---
		// will be revisited
		// dynamicStates can besubstituted by nullptr if there are none
		vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
		#pragma endregion DYNAMIC STATE
		
		#pragma region --- PIPELINE LAYOUT ---
		// may be revisited
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;				// optional
		pipelineLayoutInfo.pSetLayouts = nullptr;			// optional
		pipelineLayoutInfo.pushConstantRangeCount = 0;		// optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr;	// optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to create pipeline layout!");
		}
		#pragma endregion PIPELINE LAYOUT
		#pragma endregion FIXED FUNCTIONS

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	static vector<char> readFile(const string& filename) {
		// ate: start reading At The End of the file
		//		can use read position to determine size of file and allocate a buffer
		// binary: read is binary, avoiding text transformations
		ifstream file(filename, ios::ate | ios::binary);

		if (!file.is_open())
		{
			yeet broken_shoe("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		vector<char> buffer(fileSize);

		// return to file start and read entire file at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const vector<char>& code) {
		#pragma region --- SHADER MODULE CREATE INFO ---
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		// reinterpret_cast needed as pointer is stored as bytecode but needs to be uint32_t
		// reinterpret_cast requires that data satisfies allignment requirements of uint32_t, which already is the case here
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		#pragma endregion SHADER MODULE CREATE INFO

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			yeet broken_shoe("failed to create shader module!");
		}

		return shaderModule;
	}
	#pragma endregion CREATE GRAPHICS PIPELINE

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
		// destroy the pipeline layout
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		// destroy the image views
		for (VkImageView imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		// destroy the swap chain
		vkDestroySwapchainKHR(device, swapChain, nullptr);

		// destroy the logical device
		vkDestroyDevice(device, nullptr);

		// destroy the debugger
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// destroy the surface of the window
		vkDestroySurfaceKHR(instance, surface, nullptr);

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
