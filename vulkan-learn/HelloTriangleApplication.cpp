//
// Created by zhang on 2025/11/1.
//

#include "HelloTriangleApplication.h"

#include <filesystem>
#include <fstream>
#include <range/v3/range.hpp>
#include <queue>
#include <random>
#include <set>

namespace details
{
class VkErrorException final : public std::runtime_error
{
public:
    explicit VkErrorException(const std::string& string) : std::runtime_error(string)
    {
    };
};

// 语言选择枚举
enum class VkResultLang
{
    English,
    Chinese
};

/**
 * @brief 获取 VkResult 的详细含义描述
 *
 * @param result Vulkan 返回的 VkResult 值
 * @param lang 选择返回英文还是中文描述 (默认英文)
 * @return const char* 描述字符串
 */
inline constexpr std::string_view GetVkResultDescription(VkResult result, VkResultLang lang = VkResultLang::Chinese)
{
    bool is_zh = (lang == VkResultLang::Chinese);

    switch (result)
    {
    // ==========================================================
    // Success Codes (成功状态码)
    // ==========================================================
    case VK_SUCCESS:
        return is_zh
                   ? "命令成功完成"
                   : "Command successfully completed";
    case VK_NOT_READY:
        return is_zh
                   ? "Fence 或 Query 尚未完成"
                   : "A fence or query has not yet completed";
    case VK_TIMEOUT:
        return is_zh
                   ? "等待操作在指定时间内未完成"
                   : "A wait operation has not completed in the specified time";
    case VK_EVENT_SET:
        return is_zh
                   ? "事件(Event)已置位(Signaled)"
                   : "An event is signaled";
    case VK_EVENT_RESET:
        return is_zh
                   ? "事件(Event)已复位(Unsignaled)"
                   : "An event is unsignaled";
    case VK_INCOMPLETE:
        return is_zh
                   ? "返回数组过小，无法容纳所有结果"
                   : "A return array was too small for the result";

    // Provided by VK_KHR_swapchain
    case VK_SUBOPTIMAL_KHR:
        return is_zh
                   ? "交换链不再完全匹配 Surface 属性，但仍可成功用于显示"
                   : "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";

    // Provided by VK_KHR_deferred_host_operations
    case VK_THREAD_IDLE_KHR:
        return is_zh
                   ? "延迟操作未完成，但当前线程暂无工作"
                   : "A deferred operation is not complete but there is currently no work for this thread to do at the time of this call.";
    case VK_THREAD_DONE_KHR:
        return is_zh
                   ? "延迟操作未完成，但已无剩余工作可分配给其他线程"
                   : "A deferred operation is not complete but there is no work remaining to assign to additional threads.";
    case VK_OPERATION_DEFERRED_KHR:
        return is_zh
                   ? "请求了延迟操作，且部分工作已被推迟"
                   : "A deferred operation was requested and at least some of the work was deferred.";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return is_zh
                   ? "请求了延迟操作，但没有操作被推迟"
                   : "A deferred operation was requested and no operations were deferred.";

    // Provided by VK_VERSION_1_3
    case VK_PIPELINE_COMPILE_REQUIRED:
        // Alias: VK_PIPELINE_COMPILE_REQUIRED_EXT
        return is_zh
                   ? "请求的管线创建需要编译，但应用要求不进行编译"
                   : "A requested pipeline creation would have required compilation, but the application requested compilation to not be performed.";

    // Provided by VK_KHR_pipeline_binary
    case VK_PIPELINE_BINARY_MISSING_KHR:
        return is_zh
                   ? "尝试从内部缓存查询管线二进制文件，但缓存条目不存在"
                   : "The application attempted to create a pipeline binary by querying an internal cache, but the internal cache entry did not exist.";

    // Provided by VK_EXT_shader_object
    case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
        return is_zh
                   ? "提供的二进制 Shader 代码与此设备不兼容"
                   : "The provided binary shader code is not compatible with this device.";

    // ==========================================================
    // Error Codes (错误状态码)
    // ==========================================================
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return is_zh
                   ? "主机内存(Host Memory)分配失败"
                   : "A host memory allocation has failed.";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return is_zh
                   ? "设备内存(Device Memory)分配失败"
                   : "A device memory allocation has failed.";
    case VK_ERROR_INITIALIZATION_FAILED:
        return is_zh
                   ? "对象初始化因实现特定的原因失败"
                   : "Initialization of an object could not be completed for implementation-specific reasons.";
    case VK_ERROR_DEVICE_LOST:
        return is_zh
                   ? "逻辑或物理设备已丢失 (Device Lost)"
                   : "The logical or physical device has been lost.";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return is_zh
                   ? "内存对象映射失败"
                   : "Mapping of a memory object has failed.";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return is_zh
                   ? "请求的 Layer 不存在或无法加载"
                   : "A requested layer is not present or could not be loaded.";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return is_zh
                   ? "请求的扩展不支持"
                   : "A requested extension is not supported.";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return is_zh
                   ? "请求的特性(Feature)不支持"
                   : "A requested feature is not supported.";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return is_zh
                   ? "驱动程序不支持请求的 Vulkan 版本或不兼容"
                   : "The requested version of Vulkan is not supported by the driver or is otherwise incompatible.";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return is_zh
                   ? "此类对象的创建数量已达上限"
                   : "Too many objects of the type have already been created.";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return is_zh
                   ? "设备不支持请求的格式"
                   : "A requested format is not supported on this device.";
    case VK_ERROR_FRAGMENTED_POOL:
        return is_zh
                   ? "由于池内存碎片化，分配失败"
                   : "A pool allocation has failed due to fragmentation of the pool's memory.";

    // Provided by VK_KHR_surface
    case VK_ERROR_SURFACE_LOST_KHR:
        return is_zh
                   ? "Surface 不再可用"
                   : "A surface is no longer available.";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return is_zh
                   ? "请求的窗口已被 Vulkan 或其他 API 占用"
                   : "The requested window is already in use by Vulkan or another API.";

    // Provided by VK_KHR_swapchain
    case VK_ERROR_OUT_OF_DATE_KHR:
        return is_zh
                   ? "Surface 已变更且与交换链不兼容 (Out of Date)"
                   : "A surface has changed in such a way that it is no longer compatible with the swapchain.";

    // Provided by VK_KHR_display_swapchain
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return is_zh
                   ? "交换链使用的显示设备不兼容"
                   : "The display used by a swapchain does not use the same presentable image layout, or is incompatible.";

    // Provided by VK_NV_glsl_shader
    case VK_ERROR_INVALID_SHADER_NV:
        return is_zh
                   ? "一个或多个 Shader 编译或链接失败"
                   : "One or more shaders failed to compile or link.";

    // Provided by VK_VERSION_1_1
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        // Alias: VK_ERROR_OUT_OF_POOL_MEMORY_KHR
        return is_zh
                   ? "池内存分配失败"
                   : "A pool memory allocation has failed.";

    // Provided by VK_VERSION_1_1
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        // Alias: VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR
        return is_zh
                   ? "外部句柄无效"
                   : "An external handle is not a valid handle of the specified type.";

    // Provided by VK_VERSION_1_2
    case VK_ERROR_FRAGMENTATION:
        // Alias: VK_ERROR_FRAGMENTATION_EXT
        return is_zh
                   ? "由于碎片化，描述符池创建失败"
                   : "A descriptor pool creation has failed due to fragmentation.";

    // Provided by VK_VERSION_1_2
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        // Alias: VK_ERROR_INVALID_DEVICE_ADDRESS_EXT
        // Alias: VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR
        return is_zh
                   ? "请求的地址不可用，导致缓冲创建或内存分配失败"
                   : "A buffer creation or memory allocation failed because the requested address is not available.";

    // Provided by VK_EXT_full_screen_exclusive
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return is_zh
                   ? "独占全屏模式丢失"
                   : "An operation on a swapchain failed as it did not have exclusive full-screen access.";

    // Provided by VK_VERSION_1_0 (Optimized out by default usually, but valid return code)
    case VK_ERROR_VALIDATION_FAILED_EXT:
        // Note: VK_ERROR_VALIDATION_FAILED_EXT is an alias for value -1000011001
        // Usually handled by layers, but defined in enum.
        return is_zh
                   ? "检测到无效用法 (Validation Failed)"
                   : "A command failed because invalid usage was detected by the implementation or a validation layer.";

    // Provided by VK_EXT_image_compression_control
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
        return is_zh
                   ? "图像创建失败，压缩所需内部资源耗尽"
                   : "An image creation failed because internal resources required for compression are exhausted.";

    // Provided by VK_KHR_video_queue
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的 VkImageUsageFlags"
                   : "The requested VkImageUsageFlags are not supported.";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的视频图像布局"
                   : "The requested video picture layout is not supported.";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持指定的视频配置操作"
                   : "A video profile operation specified via VkVideoProfileInfoKHR::videoCodecOperation is not supported.";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的视频配置格式参数"
                   : "Format parameters in a requested VkVideoProfileInfoKHR chain are not supported.";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的视频配置编解码器参数"
                   : "Codec-specific parameters in a requested VkVideoProfileInfoKHR chain are not supported.";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持指定的视频标准版本"
                   : "The specified video Std header version is not supported.";

    // Provided by VK_KHR_video_encode_queue
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
        return is_zh
                   ? "视频标准参数无效或不符合语义要求"
                   : "The specified Video Std parameters do not adhere to the syntactic or semantic requirements.";

    // Provided by VK_VERSION_1_4
    case VK_ERROR_NOT_PERMITTED:
        // Alias: VK_ERROR_NOT_PERMITTED_EXT, VK_ERROR_NOT_PERMITTED_KHR
        return is_zh
                   ? "权限不足，请求的高优先级被拒绝"
                   : "The driver implementation has denied a request to acquire a priority above the default priority.";

    // Provided by VK_KHR_pipeline_binary
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
        return is_zh
                   ? "应用程序提供的空间不足以返回所有数据"
                   : "The application did not provide enough space to return all the required data.";

    // Provided by VK_VERSION_1_0
    case VK_ERROR_UNKNOWN:
        return is_zh
                   ? "发生未知错误"
                   : "An unknown error has occurred.";

    default:
        return is_zh
                   ? "未知的 VkResult 代码"
                   : "Unknown VkResult code";
    }
}

void err_check(const VkResult result, std::string_view message)
{
    if (result != VK_SUCCESS)
    {
        throw VkErrorException(fmt::format("{}({}), {}", magic_enum::enum_name<VkResult>(result),
                                           GetVkResultDescription(result), message));
    }
}
}

namespace vk_pred
{
}

std::string details::get_project_dir()
{
    auto path = std::filesystem::current_path();
    return path.parent_path().parent_path().append("vulkan-learn").string();
}

std::vector<char> details::read_file(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        return {std::istreambuf_iterator(file), std::istreambuf_iterator<char>()};
    };
    return {};
}

bool HelloTriangleApplication::check_validation_layer_support(
    std::span<const char* const> layers = k_vulkan_validation_layers)
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

bool HelloTriangleApplication::check_device_extensions_support(const VkPhysicalDevice device,
                                                               const std::span<const char* const> extensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string_view> requiredExtensions(extensions.begin(), extensions.end());
    for (const auto& [extensionName, specVersion] : availableExtensions)
    {
        requiredExtensions.erase(extensionName);
    }

    return requiredExtensions.empty();
}

void HelloTriangleApplication::create_logical_device()
{
    const auto [graphic_index, present_index] = find_queue_families_index(physical_device_);

    if (!graphic_index.has_value() || !present_index.has_value())
        throw std::runtime_error("failed to find graphics/present families!");

    const auto graphicQueueFamilyIndex = graphic_index.value();
    const auto presentQueueFamilyIndex = present_index.value();

    std::set<uint32_t> uniqueQueueFamilies = {graphicQueueFamilyIndex, presentQueueFamilyIndex};

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (auto queueFamilies : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilies;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queue_create_infos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queue_create_infos.data();
    createInfo.queueCreateInfoCount = queue_create_infos.size();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = k_device_extensions.size();
    createInfo.ppEnabledExtensionNames = k_device_extensions.data();

    details::err_check(vkCreateDevice(physical_device_, &createInfo, nullptr, &device_),
                       "failed to create logical device!");

    fmt::println("{}", "create logicalDevice");

    vkGetDeviceQueue(device_, graphicQueueFamilyIndex, 0, &present_queue_);
    vkGetDeviceQueue(device_, presentQueueFamilyIndex, 0, &graphics_queue_);
}


HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::query_swap_chain_support(
    VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
    }

    return details;
}


std::vector<uint32_t> HelloTriangleApplication::find_queue_families_index(
    const vk::PhysicalDevice device, std::function<bool(int, const VkQueueFamilyProperties&)> pred) const
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    namespace rv = std::ranges::views;

    auto debug_info = queueFamilies | rv::transform([&](vk::QueueFamilyProperties& value)
    {

        return ::convert_flags_to_names( value.queueFlags);
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

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::find_queue_families_index(
    VkPhysicalDevice device)
{
    const auto graphic_indices = find_queue_families_index(physical_device_, vk_pred::is_vk_queue_graphics);
    const auto present_indices = find_queue_families_index(physical_device_,
                                                           std::bind(&HelloTriangleApplication::is_present_support,
                                                                     this, std::placeholders::_1,
                                                                     std::placeholders::_2));

    return QueueFamilyIndices{
        !graphic_indices.empty() ? graphic_indices.front() : std::optional<uint32_t>{},
        !present_indices.empty() ? present_indices.front() : std::optional<uint32_t>{}
    };
}

uint32_t HelloTriangleApplication::find_memory_type(uint32_t typeFilter,
                                                    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memoryProperties);
    for (int i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("Memory type does not match memory type");
}

int HelloTriangleApplication::rate_device_suitability(VkPhysicalDevice device)
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

void HelloTriangleApplication::pick_physical_device()
{
    std::vector<vk::PhysicalDevice> physical_devices = vk_instance_.enumeratePhysicalDevices();

    auto is_device_suitable = [this](const vk::PhysicalDevice device)
    {
        vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
        auto deviceFeatures = device.getFeatures();

        // 是否是独立显卡
        const bool is_discrete = deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        // 是否支持多边形着色器
        const bool is_has_geometry_shader = deviceFeatures.geometryShader;

        // 是否存在图形队列
        const bool is_have_graphics_queue = !find_queue_families_index(device, vk_pred::is_vk_queue_graphics).empty();

        // 是否支持拓展
        const bool is_extension_support = check_device_extensions_support(device, k_device_extensions);

        const auto SwapChainSupportDetails = query_swap_chain_support(device);

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

    for (const auto& device : physical_devices | std::ranges::views::filter(is_device_suitable))
    {
        int score = rate_device_suitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        physical_device_ = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    if (physical_device_ == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find suited physical device");
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_device_, &deviceProperties);


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
                                                                , deviceProperties.pipelineCacheUUID + VK_UUID_SIZE) |
                 std::ranges::views::transform([](auto& c)
                 {
                     const size_t buf_size = 10;
                     char buf[buf_size]{};
                     std::to_chars_result result = std::to_chars(buf, buf + buf_size, c, 16);
                     return std::string(buf, result.ptr - buf);
                 }) | std::ranges::views::join | std::ranges::to<std::string>());
}

void HelloTriangleApplication::populate_debug_messenger_create_info(
    vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.setMessageSeverity(
                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
              .setMessageType(
                  vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                  | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                  | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
              .setPfnUserCallback(reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debug_callback));
}

VkResult HelloTriangleApplication::create_debug_utils_messenger_ext(VkInstance instance,
                                                                    const VkDebugUtilsMessengerCreateInfoEXT*
                                                                    pCreateInfo,
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
}

void HelloTriangleApplication::destroy_debug_utils_messenger_ext(VkInstance instance,
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

void HelloTriangleApplication::setup_debug_message()
{
    if (!k_enable_validation_layers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug_callback;
    createInfo.pUserData = nullptr; // Optional

    details::err_check(create_debug_utils_messenger_ext(vk_instance_, &createInfo, nullptr, &debug_messenger_),
                       "failed to set up debug messenger!");
}

VkBool32 HelloTriangleApplication::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
        fmt::println(stderr, "[{}][{}] {}", getSeverity(), getType(), pCallbackData->pMessage);
        //std::cerr << std::format("[{}] {}", getSeverity(), pCallbackData->pMessage) << std::endl;
    }
    else
    {
        fmt::println(stdout, "[{}][{}] {}", getSeverity(), getType(), pCallbackData->pMessage);
        //std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    }


    return VK_FALSE;
}

std::vector<const char*> HelloTriangleApplication::get_required_extensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (k_enable_validation_layers)
    {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


void HelloTriangleApplication::extension_check()
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

void HelloTriangleApplication::create_instance()
{
    if (k_enable_validation_layers && !check_validation_layer_support())
    {
        throw std::runtime_error("validation layers requested but not available!");
    }
    constexpr vk::ApplicationInfo info{
        "Hello Triangle",
        VK_MAKE_VERSION(1, 0, 0),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0,
    };

    const auto extensions = get_required_extensions();

    vk::InstanceCreateInfo instance_create_info({}, &info, extensions);

    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info{};
    if (k_enable_validation_layers)
    {
        populate_debug_messenger_create_info(debug_create_info);
        instance_create_info.setPEnabledLayerNames(k_vulkan_validation_layers)
                            .setPNext(&debug_create_info);
    }

    vk_instance_ = vk::createInstance(instance_create_info);
}

VkSurfaceFormatKHR HelloTriangleApplication::choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }
    return availableFormats.front();
}

VkPresentModeKHR HelloTriangleApplication::choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApplication::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void HelloTriangleApplication::create_vertex_buffer()
{
    auto buffer_size = sizeof(decltype(vertices)::value_type) * vertices.size();

    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging_buffer, staging_buffer_memory);

    // 第五步：填充顶点数据（CPU -> GPU）
    void* data;
    vkMapMemory(device_, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(device_, staging_buffer_memory);

    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  vertex_buffer_, vertex_buffer_memory_);

    copy_buffer(staging_buffer, vertex_buffer_, buffer_size);

    vkDestroyBuffer(device_, staging_buffer, nullptr);
    vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void HelloTriangleApplication::create_index_buffer()
{
    auto buffer_size = sizeof(decltype(indices)::value_type) * indices.size();

    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging_buffer, staging_buffer_memory);

    // 第五步：填充顶点数据（CPU -> GPU）
    void* data;
    vkMapMemory(device_, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), buffer_size);
    vkUnmapMemory(device_, staging_buffer_memory);

    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  index_buffer_, index_buffer_memory_);

    copy_buffer(staging_buffer, index_buffer_, buffer_size);

    vkDestroyBuffer(device_, staging_buffer, nullptr);
    vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void HelloTriangleApplication::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                             VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                             VkDeviceMemory& buffer_memory)
{
    // 第一步：创建缓冲区对象 (VkBuffer)
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = size;

    details::err_check(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer), "failed to create vertex buffer!");

    // 第二步：查询内存需求
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

    // 第三步：选择并分配设备内存
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    details::err_check(vkAllocateMemory(device_, &allocInfo, nullptr, &buffer_memory),
                       "failed to allocate vertex buffer memory!");

    // 第四步：将内存绑定到缓冲区
    vkBindBufferMemory(device_, buffer, buffer_memory, 0);
}

void HelloTriangleApplication::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copyRegion);
    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue_);
}

void HelloTriangleApplication::create_swap_chain()
{
    const auto [capabilities, formats, present_modes] = query_swap_chain_support(physical_device_);
    auto [format, color_space] = choose_swap_surface_format(formats);
    const VkPresentModeKHR presentMode = choose_swap_present_mode(present_modes);
    const VkExtent2D extent = choose_swap_extent(capabilities);

    uint32_t image_count = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface_;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format;
    create_info.imageColorSpace = color_space;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    if (const auto& [graphic_index, present_index] = find_queue_families_index(physical_device_); graphic_index !=
        present_index)
    {
        std::array<uint32_t, 2> queueFamilies{graphic_index.value(), present_index.value()};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilies.data();
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    }
    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    create_info.presentMode = presentMode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    details::err_check(vkCreateSwapchainKHR(device_, &create_info, nullptr, &swap_chain_),
                       "Failed to create a swapchain");

    vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, nullptr);
    swap_chain_images_.resize(image_count);
    vkGetSwapchainImagesKHR(device_, swap_chain_, &image_count, swap_chain_images_.data());


    swap_chain_image_format_ = format;
    swap_chain_extent_ = extent;
}

void HelloTriangleApplication::create_image_view()
{
    swap_chain_image_views_.resize(swap_chain_images_.size());

    for (size_t i = 0; i < swap_chain_images_.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swap_chain_images_[i];

        // viewType 和 format 字段指定应如何解释图像数据。
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swap_chain_image_format_;

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

        details::err_check(vkCreateImageView(device_, &createInfo, nullptr, &swap_chain_image_views_[i]),
                           "Failed to create a image view");
    }
}

void HelloTriangleApplication::create_graphics_pipeline()
{
    auto frag_spv = details::read_file(details::get_project_dir() + "/shader/sample_triangle.frag.spv");
    auto vert_spv = details::read_file(details::get_project_dir() + "/shader/sample_triangle.vert.spv");

    if (frag_spv.empty() || vert_spv.empty())
        throw std::runtime_error("Could not load shaders");

    VkShaderModule vertShaderModule = create_shader_module(vert_spv);
    VkShaderModule fragShaderModule = create_shader_module(frag_spv);

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


    auto bindingDescription = VkBindingDescription<Vertex>::get_binding_description();
    auto attributeDescriptions = VkBindingDescription<Vertex>::get_attribute_descriptions();
    // 顶点输入
    // `VkPipelineVertexInputStateCreateInfo` 结构描述了将传递给顶点着色器的顶点数据的格式。
    // 它大致通过两种方式描述：
    // 绑定：数据之间的间距以及数据是每个顶点还是每个实例
    // 属性描述：传递给顶点着色器的属性类型，从哪个绑定加载它们以及在哪个偏移处加载
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
    viewport.width = swap_chain_extent_.width;
    viewport.height = swap_chain_extent_.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {swap_chain_extent_.width, swap_chain_extent_.height};

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

    details::err_check(vkCreatePipelineLayout(device_, &pipelineLayoutCreateInfo, nullptr, &pipeline_layout_),
                       "failed to create pipeline layout");


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

    pipelineCreateInfo.layout = pipeline_layout_;

    pipelineCreateInfo.renderPass = render_pass_;
    pipelineCreateInfo.subpass = 0;

    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineCreateInfo.basePipelineIndex = -1; // Optional

    details::err_check(
        vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphics_pipeline_),
        "failed to create pipeline");

    vkDestroyShaderModule(device_, vertShaderModule, nullptr);
    vkDestroyShaderModule(device_, fragShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::create_shader_module(std::span<char> code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    details::err_check(vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule),
                       "Failed to create shader module");

    return shaderModule;
}

void HelloTriangleApplication::create_render_pass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swap_chain_image_format_;
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

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;

    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    details::err_check(vkCreateRenderPass(device_, &renderPassCreateInfo, nullptr, &render_pass_),
                       "Failed to create render pass !");
}

void HelloTriangleApplication::create_framebuffers()
{
    swapChainFramebuffers.resize(swap_chain_image_views_.size(), nullptr);

    for (auto i = 0; i < swap_chain_image_views_.size(); i++)
    {
        VkImageView attachments[] = {
            swap_chain_image_views_[i]
        };
        VkFramebufferCreateInfo frameBufferCreateInfo{};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass = render_pass_;
        frameBufferCreateInfo.attachmentCount = 1;
        frameBufferCreateInfo.pAttachments = attachments;
        frameBufferCreateInfo.width = swap_chain_extent_.width;
        frameBufferCreateInfo.height = swap_chain_extent_.height;
        frameBufferCreateInfo.layers = 1;

        details::err_check(vkCreateFramebuffer(device_, &frameBufferCreateInfo, nullptr, &swapChainFramebuffers[i]),
                           "Failed to create framebuffer !");
    }
}

void HelloTriangleApplication::create_command_pool()
{
    auto [graphicsFamily, presentFamily] = find_queue_families_index(physical_device_);

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = graphicsFamily.value();

    details::err_check(vkCreateCommandPool(device_, &commandPoolCreateInfo, nullptr, &command_pool_),
                       "Failed to create command pool !");
}

void HelloTriangleApplication::create_command_buffer()
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = command_pool_;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    command_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
    details::err_check(vkAllocateCommandBuffers(device_, &commandBufferAllocateInfo, command_buffers_.data()),
                       "Failed to create command buffer command buffers !");
}

void HelloTriangleApplication::record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    details::err_check(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin record command buffer!");

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(swap_chain_extent_.width);
    viewport.height = static_cast<float>(swap_chain_extent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass_;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swap_chain_extent_;

    VkClearValue clearValue{{{0.0, 0.0, 0.0, 1.0}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 绑定图形管线
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent_;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertex_buffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    for (auto i = 0; i < 1; ++i)
    {
        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }


    vkCmdEndRenderPass(commandBuffer);

    details::err_check(vkEndCommandBuffer(commandBuffer), "failed to record command buffer!");
}

void HelloTriangleApplication::create_sync_object()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    fences_in_flight_.resize(MAX_FRAMES_IN_FLIGHT);

    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        details::err_check(vkCreateSemaphore(device_, &semaphoreCreateInfo, nullptr, &image_available_semaphores_[i]),
                           "Failed to create semaphore !");
        details::err_check(vkCreateFence(device_, &fenceCreateInfo, nullptr, &fences_in_flight_[i]),
                           "Failed to create fence !");
    }

    render_finished_semaphores_.resize(swap_chain_images_.size());
    // https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html
    for (auto i = 0; i < swap_chain_images_.size(); ++i)
    {
        details::err_check(vkCreateSemaphore(device_, &semaphoreCreateInfo, nullptr, &render_finished_semaphores_[i]),
                           "Failed to create semaphore !");
    }
}

void HelloTriangleApplication::cleanup_swap_chain() const
{
    for (const auto swapChainFramebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device_, swapChainFramebuffer, nullptr);
    }
    for (const auto imageView : swap_chain_image_views_)
    {
        vkDestroyImageView(device_, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
}

void HelloTriangleApplication::recreate_swap_chain()
{
    vkDeviceWaitIdle(device_);
    cleanup_swap_chain();
    create_swap_chain();
    create_image_view();
    create_framebuffers();
}

void HelloTriangleApplication::draw_frame()
{
    auto current_flight_fence = fences_in_flight_[current_flight_frame_];
    vkWaitForFences(device_, 1, &current_flight_fence, VK_TRUE, UINT64_MAX);


    VkSemaphore current_available_semaphore = image_available_semaphores_[current_flight_frame_];
    uint32_t image_index = 0;
    auto result = vkAcquireNextImageKHR(device_, swap_chain_, UINT64_MAX,
                                        current_available_semaphore, VK_NULL_HANDLE,
                                        &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swap_chain();
        return; // 重新创建交换链后，跳过当前帧
    }

    else if (result == VK_SUBOPTIMAL_KHR)
    {
        // 不理想但可用，可以继续
    }
    else
    {
        details::err_check(result, "failed to acquire swap chain image!");
    }
    // 使用image对应的fence

    auto current_command_buffer = command_buffers_[current_flight_frame_];
    auto current_render_finished_semaphore = render_finished_semaphores_[image_index];

    //fmt::print("[{}] image_index: {} flight in {} ", frame_count_, image_index, current_flight_frame_);

    vkResetCommandBuffer(current_command_buffer, 0);
    record_command_buffer(current_command_buffer, image_index);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &current_available_semaphore;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &current_command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &current_render_finished_semaphore;

    // 使用之前先使fence处于未触发状态
    vkResetFences(device_, 1, &current_flight_fence);

    details::err_check(vkQueueSubmit(graphics_queue_, 1, &submitInfo, current_flight_fence),
                       "Failed to submit command buffer event !");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &current_render_finished_semaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swap_chain_;
    presentInfo.pImageIndices = &image_index;
    vkQueuePresentKHR(present_queue_, &presentInfo);

    ++frame_count_;
    current_flight_frame_ = (current_flight_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApplication::cleanup()
{
    vkDestroyBuffer(device_, vertex_buffer_, nullptr);
    vkFreeMemory(device_, vertex_buffer_memory_, nullptr);
    vkDestroyBuffer(device_, index_buffer_, nullptr);
    vkFreeMemory(device_, index_buffer_memory_, nullptr);
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
        vkDestroyFence(device_, fences_in_flight_[i], nullptr);
    }
    for (auto i = 0; i < swap_chain_images_.size(); i++)
    {
        vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
    }

    vkDestroyCommandPool(device_, command_pool_, nullptr);

    cleanup_swap_chain();
    vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
    vkDestroyRenderPass(device_, render_pass_, nullptr);

    vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);
    vkDestroyDevice(device_, nullptr);
    if (k_enable_validation_layers)
    {
        destroy_debug_utils_messenger_ext(vk_instance_, debug_messenger_, nullptr);
    }
    vkDestroyInstance(vk_instance_, nullptr);
    glfwDestroyWindow(window_);
    glfwTerminate();
}
