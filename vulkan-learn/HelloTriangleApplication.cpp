//
// Created by zhang on 2025/11/1.
//

#include "HelloTriangleApplication.h"

#include <fstream>
#include <print>
#include <set>

namespace vk_pred {

}

std::string details::get_project_dir() {
    return "E:/workspace/ZetaEngine/vulkan-learn";
}

std::vector<char> details::readFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    };
    return {};
}

bool HelloTriangleApplication::checkValidationLayerSupport(std::span<const char* const> layers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::unordered_set<std::string_view> availableLayersName;

    for (const auto& availableLayer : availableLayers)
    {
        availableLayersName.emplace(availableLayer.layerName);
    }

    if (std::ranges::any_of(layers, [&](const auto& layerName)
    {
        return !availableLayersName.contains(layerName);
    }))
    {
        return false;
    }
    return true;
};

bool HelloTriangleApplication::checkDeviceExtensionsSupport(const VkPhysicalDevice device,const std::span<const char* const> extensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string_view> requiredExtensions(extensions.begin(), extensions.end());
    for (const auto&[extensionName, specVersion] : availableExtensions) {
        requiredExtensions.erase(extensionName);
    }

    return requiredExtensions.empty();
}
void HelloTriangleApplication::createLogicalDevice()
{
    const auto [graphic_index, present_index] = findQueueFamiliesIndex(physicalDevice);

    if (!graphic_index.has_value() || !present_index.has_value())
        throw std::runtime_error("failed to find graphics/present families!");

    const auto graphicQueueFamilyIndex = graphic_index.value();
    const auto presentQueueFamilyIndex = present_index.value();

    std::set<uint32_t> uniqueQueueFamilies = {graphicQueueFamilyIndex, presentQueueFamilyIndex};

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto queueFamilies: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }else
    {
        std::println("{}","create logicalDevice");
    }

    vkGetDeviceQueue(logicalDevice, graphicQueueFamilyIndex, 0, &presentQueue);
    vkGetDeviceQueue(logicalDevice, presentQueueFamilyIndex, 0, &graphicsQueue);
}


HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

std::vector<uint32_t> HelloTriangleApplication::findQueueFamiliesIndex(const VkPhysicalDevice device, std::function<bool(int, const VkQueueFamilyProperties&)> pred)
{
    // Assign index to queue families that could be found
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    namespace rv = std::ranges::views;
    auto debug_info = queueFamilies | rv::transform([&](auto& value)
    {
        return GetFlagBitsDisplay<VkQueueFlagBits>(value.queueFlags);
    }) | std::ranges::to<std::vector<std::string>>();

    std::vector<int> vec = {10, 20, 30, 40, 50};



    auto Families = queueFamilies | rv::enumerate
    | rv::filter([&pred](auto&& pair)
    {
        return std::invoke(pred, std::get<0>(pair), std::get<1>(pair));
    }) | rv::transform([](auto&& value)
    {
        return std::get<0>(value);
    }) | std::ranges::to<std::vector<uint32_t>>();


    return Families;
}

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::findQueueFamiliesIndex(VkPhysicalDevice device) {
    const auto graphic_indices = findQueueFamiliesIndex(physicalDevice, vk_pred::is_vk_queue_graphics);
    const auto present_indices = findQueueFamiliesIndex(physicalDevice, std::bind(&HelloTriangleApplication::is_present_support, this, std::placeholders::_1, std::placeholders::_2));

    return QueueFamilyIndices{!graphic_indices.empty() ? graphic_indices.front() : std::optional<uint32_t>{}
        , !present_indices.empty() ? present_indices.front() : std::optional<uint32_t>{}};
}

int HelloTriangleApplication::rateDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader)
    {
        return 0;
    }

    return score;
}

void HelloTriangleApplication::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find suited physical device");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    auto isDeviceSuitable = [this](const VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // 是否是独立显卡
        const bool is_discrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        // 是否支持多边形着色器
        const bool is_has_geometry_shader = deviceFeatures.geometryShader;

        // 是否存在图形队列
        const bool is_have_graphics_queue = !findQueueFamiliesIndex(device, vk_pred::is_vk_queue_graphics).empty();

        // 是否支持拓展
        const bool is_extension_support = checkDeviceExtensionsSupport(device, deviceExtensions);

        const auto SwapChainSupportDetails = querySwapChainSupport(device);

        // 至少有一个受支持的图像格式和一个受支持的演示模式, 交换链支持就足够了
        const bool swapChainAdequate = !SwapChainSupportDetails.formats.empty()
            && !SwapChainSupportDetails.presentModes.empty();

        return is_discrete
        && is_has_geometry_shader
        && is_have_graphics_queue
        && is_extension_support
        && swapChainAdequate;
    };

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices | std::ranges::views::filter(isDeviceSuitable))
    {
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find suited physical device");
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);


    auto number_to_string = [](auto& c, int base = 10)
    {
        constexpr size_t buf_size = 10;
        char buf[buf_size]{};
        const std::to_chars_result result = std::to_chars(buf, buf + buf_size, c, base);
        return std::string(buf, result.ptr - buf);
    };

    fmt::println("{}", "Select GPU:");
    fmt::println("apiVersion        : {}", std::vector<uint32_t>({
                     VK_VERSION_MAJOR(deviceProperties.apiVersion),
                     VK_VERSION_MINOR(deviceProperties.apiVersion),
                     VK_VERSION_PATCH(deviceProperties.apiVersion),
                 }) | std::ranges::views::transform(number_to_string)
                 | std::ranges::views::join_with('.') | std::ranges::to<std::string>());


    fmt::println("driverVersion     : {}", std::vector<uint32_t>({
                     VK_VERSION_MAJOR(deviceProperties.driverVersion),
                     (deviceProperties.driverVersion >> 14) & 0x3FF,
                     deviceProperties.driverVersion & 0x3FFF,
                 }) | std::ranges::views::transform(std::bind(number_to_string, std::placeholders::_1, 16))
                 | std::ranges::views::join_with('.') | std::ranges::to<std::string>());

    fmt::println("vendorID          : {}", deviceProperties.vendorID);
    fmt::println("deviceID          : {}", deviceProperties.deviceID);
    fmt::println("deviceType        : {}", magic_enum::enum_name(deviceProperties.deviceType));
    fmt::println("deviceName        : {}", deviceProperties.deviceName);
    fmt::println("pipelineCacheUUID : {}", std::vector<uint8_t>(deviceProperties.pipelineCacheUUID
                , deviceProperties.pipelineCacheUUID + VK_UUID_SIZE) | std::ranges::views::transform([](auto& c)
                 {
                     const size_t buf_size = 10;
                     char buf[buf_size]{};
                     std::to_chars_result result = std::to_chars(buf, buf + buf_size, c, 16);
                     return std::string(buf, result.ptr - buf);
                 }) | std::ranges::views::join | std::ranges::to<std::string>());
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult HelloTriangleApplication::CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    //return vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                             VkDebugUtilsMessengerEXT debugMessenger,
                                                             const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void HelloTriangleApplication::setupDebugMessage()
{
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
    if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

VkBool32 HelloTriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData)
{
    auto getSeverity = [messageSeverity]()
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "Verbose";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "Info";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "Warning";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "Error";
        default: return "Unknow";
        }
    };
    auto getType = [messageType]()
    {
        std::vector<std::string> types;
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        {
            types.emplace_back("General");
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            types.emplace_back("Validation");
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            types.emplace_back("Performance");
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
        {
            types.emplace_back("Dab");
        }
        return types | std::ranges::views::join_with('|') | std::ranges::to<std::string>();
    };
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::println(std::cerr, "[{}][{}] {}", getSeverity(), getType(), pCallbackData->pMessage);
        //std::cerr << std::format("[{}] {}", getSeverity(), pCallbackData->pMessage) << std::endl;
    }
    else
    {
        std::println(std::cout, "[{}][{}] {}", getSeverity(), getType(), pCallbackData->pMessage);
        //std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    }


    return VK_FALSE;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


void HelloTriangleApplication::extensionCheck()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::cout << "available extensions:\n";

    for (const auto& extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void HelloTriangleApplication::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
    {
        throw std::runtime_error("validation layers requested but not available!");
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    const auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();


    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (const auto Result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
        Result != VK_SUCCESS)
    {
        throw std::runtime_error(std::string(magic_enum::enum_name(Result)) + ", failed to create instance!");
    };
}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }
    return availableFormats.front();
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto& presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void HelloTriangleApplication::createSwapChain() {
    const auto [capabilities, formats, presentModes] = querySwapChainSupport(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
    VkExtent2D extent = chooseSwapExtent(capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    if (const auto& [graphic_index, present_index] = findQueueFamiliesIndex(physicalDevice); graphic_index != present_index) {
        std::array<uint32_t, 2> queueFamilies{graphic_index.value(), present_index.value()};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilies.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a swapchain");
    }

    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());


    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void HelloTriangleApplication::createImageView() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];

        // viewType 和 format 字段指定应如何解释图像数据。
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // 字段允许您对颜色通道进行混合。例如，您可以将所有通道映射到单色纹理的红色通道。您还可以将常量值 0 和 1 映射到通道。在我们的例子中，我们将坚持默认映射。
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // subresourceRange 字段描述了图像的用途以及应访问图像的哪一部分。我们的图像将用作颜色目标，没有任何 mipmap 级别或多个图层。
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a image view");
        }

    }
}

void HelloTriangleApplication::createGraphicsPipeline() {
    auto frag_spv = details::readFile(details::get_project_dir() + "shader/sample_trangle.frag.spv");
    auto vert_spv = details::readFile(details::get_project_dir() + "shader/sample_trangle.vert.spv");

    VkShaderModule vertShaderModule = createShaderModule(vert_spv);
    VkShaderModule fragShaderModule = createShaderModule(frag_spv);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";
    VkPipelineShaderStageCreateInfo fragShaderCreateInfo{};
    fragShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderCreateInfo.module = fragShaderModule;
    fragShaderCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo[] = {vertShaderStageCreateInfo, fragShaderCreateInfo};



    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::createShaderModule(std::span<char> code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}
