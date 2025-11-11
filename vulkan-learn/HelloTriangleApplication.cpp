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
    return "E:/workspace/vulkan-learn/vulkan-learn";
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
    auto frag_spv = details::readFile(details::get_project_dir() + "/shader/sample_triangle.frag.spv");
    auto vert_spv = details::readFile(details::get_project_dir() + "/shader/sample_triangle.vert.spv");

    if (frag_spv.empty() || vert_spv.empty())
        throw std::runtime_error("Could not load shaders");

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

    // 顶点输入
    // `VkPipelineVertexInputStateCreateInfo` 结构描述了将传递给顶点着色器的顶点数据的格式。
    // 它大致通过两种方式描述：
    // 绑定：数据之间的间距以及数据是每个顶点还是每个实例
    // 属性描述：传递给顶点着色器的属性类型，从哪个绑定加载它们以及在哪个偏移处加载
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

    // 输入汇编
    // `VkPipelineInputAssemblyStateCreateInfo` 结构描述了两件事：将从顶点绘制的几何图形类型以及是否应启用基元重启。
    // 前者在 `topology` 成员中指定，并且可以具有如下值：
    // `VK_PRIMITIVE_TOPOLOGY_POINT_LIST`：来自顶点的点
    // `VK_PRIMITIVE_TOPOLOGY_LINE_LIST`：每 2 个顶点之间的直线，不复用
    // `VK_PRIMITIVE_TOPOLOGY_LINE_STRIP`：每条线的结束顶点用作下一条线的起始顶点
    // `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST`：每 3 个顶点组成的三角形，不复用
    // `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP`：每个三角形的第二个和第三个顶点用作下一个三角形的前两个顶点

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 视口和剪裁矩形
    // 视口基本上描述了输出将渲染到的帧缓冲区的区域。这几乎总是 `(0, 0)` 到 `(宽度, 高度)`，在本教程中也将是这种情况。
    // 交换链及其图像的大小可能与窗口的 `WIDTH` 和 `HEIGHT` 不同。交换链图像稍后将用作帧缓冲区，因此我们应坚持使用它们的大小。
    // `minDepth` 和 `maxDepth` 值指定用于帧缓冲区的深度值的范围。这些值必须在 `[0.0f, 1.0f]` 范围内，但是 `minDepth` 可能高于 `maxDepth`。
    // 如果您没有做任何特殊的事情，则应坚持使用 `0.0f` 和 `1.0f` 的标准值。
    // 视口定义从图像到帧缓冲区的转换，而剪裁矩形定义了实际存储像素的区域。任何超出剪裁矩形范围的像素都将被光栅化器丢弃。它们的作用类似于过滤器而不是转换。
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { swapChainExtent.width, swapChainExtent.height };

    // 视口和剪裁矩形可以指定为管线的静态部分，也可以指定为命令缓冲区中设置的动态状态。
    // 尽管前者更符合其他状态，但将视口和剪裁状态设置为动态通常很方便，因为它为您提供了更大的灵活性。
    // 这非常常见，并且所有实现都可以处理这种动态状态而不会造成性能损失。
    // 当选择动态视口和剪裁矩形时，您需要为管线启用相应的动态状态
    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamicState.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    //viewportState.pViewports = &viewport;
    //viewportState.pScissors = &scissor;

    // 光栅化器
    // 光栅化器获取顶点着色器中顶点形成的几何形状，并将其转换为片段，以便由片段着色器着色。它还执行深度测试、背面剔除和裁剪测试
    // ，并且可以配置为输出填充整个多边形的片段或仅输出边缘（线框渲染）。
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // 如果将 depthClampEnable 设置为 VK_TRUE，则超出近平面和远平面的片段将被钳制到它们，而不是被丢弃。
    // 这在某些特殊情况下（如阴影贴图）很有用。使用此功能需要启用 GPU 功能。
    rasterizer.depthClampEnable = VK_FALSE;
    // 如果将 rasterizerDiscardEnable 设置为 VK_TRUE，则几何体永远不会通过光栅化器阶段。这基本上禁用了对帧缓冲区的任何输出。
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // polygonMode 确定如何为几何图形生成片段。以下模式可用
    //  VK_POLYGON_MODE_FILL：用片段填充多边形的区域
    //  VK_POLYGON_MODE_LINE：多边形边缘绘制为线条
    //  VK_POLYGON_MODE_POINT：多边形顶点绘制为点
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    // lineWidth 成员很简单，它以片段数量描述线条的粗细。支持的最大线宽取决于硬件，任何粗于 1.0f 的线都需要启用 wideLines GPU 功能。
    rasterizer.lineWidth = 1.0f;

    // cullMode 变量确定要使用的面剔除类型。您可以禁用剔除，剔除正面、剔除背面或两者都剔除。
    // frontFace 变量指定被认为是正面的面的顶点顺序，可以是顺时针或逆时针。
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // 光栅化器可以通过添加常量值或根据片段的斜率来偏移深度值。
    // 这有时用于阴影映射，但我们不会使用它。只需将 depthBiasEnable 设置为 VK_FALSE 即可。
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // 多重采样
    // VkPipelineMultisampleStateCreateInfo 结构体配置多重采样，这是执行抗锯齿的方法之一。
    // 它的工作原理是组合光栅化到同一像素的多个多边形的片段着色器结果。这种情况主要发生在边缘，这也是最明显的锯齿伪影发生的地方。
    // 由于如果只有一个多边形映射到像素，则它不需要多次运行片段着色器，因此它的成本远低于简单地渲染到更高的分辨率然后缩小。启用它需要启用 GPU 功能。
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional
    // 暂时保持禁用状态。

    // 颜色混合
    // 在片段着色器返回颜色后，需要将其与帧缓冲区中已有的颜色组合。这种转换称为颜色混合，有两种方法可以实现：
    //  混合旧值和新值以生成最终颜色
    //  使用按位运算组合旧值和新值
    // 有两种结构体用于配置颜色混合。第一个结构体 VkPipelineColorBlendAttachmentState 包含每个附加帧缓冲区的配置，第二个结构体 VkPipelineColorBlendStateCreateInfo 包含全局颜色混合设置。
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    //  if (blendEnable) {
    //      finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
    //      finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
    //  } else {
    //      finalColor = newColor;
    //  }
    //
    //  finalColor = finalColor & colorWriteMask;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // 第二个结构体引用所有帧缓冲区的结构体数组，并允许您设置混合常量，这些常量可以用作上述计算中的混合因子。
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0; // Optional
    colorBlending.blendConstants[1] = 0; // Optional
    colorBlending.blendConstants[2] = 0; // Optional
    colorBlending.blendConstants[3] = 0; // Optional


    // 管线布局
    // 您可以在着色器中使用 uniform 值，它们是类似于动态状态变量的全局变量，可以在绘制时更改以改变着色器的行为，而无需重新创建它们。
    // 它们通常用于将变换矩阵传递给顶点着色器，或在片段着色器中创建纹理采样器。
    // 这些 uniform 值需要在创建管线期间通过创建 VkPipelineLayout 对象来指定。
    // 即使我们要在未来的章节中使用它们，我们仍然需要创建一个空的管线布局。

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }


    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStageCreateInfo;

    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizer;
    pipelineCreateInfo.pMultisampleState = &multisampling;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlending;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    pipelineCreateInfo.layout = pipelineLayout;

    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;

    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineCreateInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline");
    }

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

void HelloTriangleApplication::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // loadOp 和 storeOp 决定在渲染之前和渲染之后如何处理附件中的数据。loadOp 有以下选择
    //  VK_ATTACHMENT_LOAD_OP_LOAD：保留附件的现有内容
    //  VK_ATTACHMENT_LOAD_OP_CLEAR：在开始时将值清除为常量
    //  VK_ATTACHMENT_LOAD_OP_DONT_CARE：现有内容未定义；我们不在乎它们
    // 对于 storeOp 只有两种可能性
    //  VK_ATTACHMENT_STORE_OP_STORE：渲染的内容将存储在内存中，并且可以稍后读取
    //  VK_ATTACHMENT_STORE_OP_DONT_CARE：渲染操作后帧缓冲区的内容将未定义
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // loadOp 和 storeOp 应用于颜色和深度数据，而 stencilLoadOp / stencilStoreOp 应用于模板数据。我们的应用程序不会对模板缓冲区执行任何操作，因此加载和存储的结果无关紧要。
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 子过程和附件引用
    VkAttachmentReference colorAttachmentReference{};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;


    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass !");
    }



}

void HelloTriangleApplication::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size(), nullptr);

    for (auto i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };
        VkFramebufferCreateInfo frameBufferCreateInfo{};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass = renderPass;
        frameBufferCreateInfo.attachmentCount = 1;
        frameBufferCreateInfo.pAttachments = attachments;
        frameBufferCreateInfo.width = swapChainExtent.width;
        frameBufferCreateInfo.height = swapChainExtent.height;
        frameBufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer !");
        }
    }
}

void HelloTriangleApplication::createCommandPool() {
    auto [graphicsFamily, presentFamily] = findQueueFamiliesIndex(physicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = graphicsFamily.value();

    if (vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool !");
    }
}

void HelloTriangleApplication::createCommandBuffer() {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command buffer command buffers !");
    }
}

void HelloTriangleApplication::cleanup() {

    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    for (const auto swapChainFramebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(logicalDevice, swapChainFramebuffer, nullptr);
    }

    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
    for (const auto imageView : swapChainImageViews) {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    vkDestroySurfaceKHR(vkInstance, surface, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
    }
    vkDestroyInstance(vkInstance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
