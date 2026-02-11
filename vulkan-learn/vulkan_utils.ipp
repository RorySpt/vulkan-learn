namespace vulkan_utils
{
constexpr std::string_view GetVkResultDescription(VkResult result,
                                                  VkResultLang lang)
{
    bool is_zh = (lang == VkResultLang::Chinese);

    switch (result)
    {
    // ==========================================================
    // Success Codes (成功状态码)
    // ==========================================================
    case VK_SUCCESS:
        return is_zh ? "命令成功完成" : "Command successfully completed";
    case VK_NOT_READY:
        return is_zh
                   ? "Fence 或 Query 尚未完成"
                   : "A fence or query has not yet completed";
    case VK_TIMEOUT:
        return is_zh
                   ? "等待操作在指定时间内未完成"
                   : "A wait operation has not completed in the "
                   "specified time";
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
                   : "A swapchain no longer matches the surface "
                   "properties exactly, but can still be used to "
                   "present to the surface successfully.";

    // Provided by VK_KHR_deferred_host_operations
    case VK_THREAD_IDLE_KHR:
        return is_zh
                   ? "延迟操作未完成，但当前线程暂无工作"
                   : "A deferred operation is not complete but there is "
                   "currently no work for this thread to do at the "
                   "time of this call.";
    case VK_THREAD_DONE_KHR:
        return is_zh
                   ? "延迟操作未完成，但已无剩余工作可分配给其他线程"
                   : "A deferred operation is not complete but there is "
                   "no work remaining to assign to additional threads.";
    case VK_OPERATION_DEFERRED_KHR:
        return is_zh
                   ? "请求了延迟操作，且部分工作已被推迟"
                   : "A deferred operation was requested and at least "
                   "some of the work was deferred.";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return is_zh
                   ? "请求了延迟操作，但没有操作被推迟"
                   : "A deferred operation was requested and no "
                   "operations were deferred.";

    // Provided by VK_VERSION_1_3
    case VK_PIPELINE_COMPILE_REQUIRED:
        // Alias: VK_PIPELINE_COMPILE_REQUIRED_EXT
        return is_zh
                   ? "请求的管线创建需要编译，但应用要求不进行编译"
                   : "A requested pipeline creation would have required "
                   "compilation, but the application requested "
                   "compilation to not be performed.";

    // Provided by VK_KHR_pipeline_binary
    case VK_PIPELINE_BINARY_MISSING_KHR:
        return is_zh
                   ? "尝试从内部缓存查询管线二进制文件，但缓存条目不存在"
                   : "The application attempted to create a pipeline "
                   "binary by querying an internal cache, but the "
                   "internal cache entry did not exist.";

    // Provided by VK_EXT_shader_object
    case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
        return is_zh
                   ? "提供的二进制 Shader 代码与此设备不兼容"
                   : "The provided binary shader code is not compatible "
                   "with this device.";

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
                   : "Initialization of an object could not be completed "
                   "for implementation-specific reasons.";
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
                   : "A requested layer is not present or could not be "
                   "loaded.";
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
                   : "The requested version of Vulkan is not supported "
                   "by the driver or is otherwise incompatible.";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return is_zh
                   ? "此类对象的创建数量已达上限"
                   : "Too many objects of the type have already been "
                   "created.";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return is_zh
                   ? "设备不支持请求的格式"
                   : "A requested format is not supported on this "
                   "device.";
    case VK_ERROR_FRAGMENTED_POOL:
        return is_zh
                   ? "由于池内存碎片化，分配失败"
                   : "A pool allocation has failed due to fragmentation "
                   "of the pool's memory.";

    // Provided by VK_KHR_surface
    case VK_ERROR_SURFACE_LOST_KHR:
        return is_zh
                   ? "Surface 不再可用"
                   : "A surface is no longer available.";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return is_zh
                   ? "请求的窗口已被 Vulkan 或其他 API 占用"
                   : "The requested window is already in use by Vulkan "
                   "or another API.";

    // Provided by VK_KHR_swapchain
    case VK_ERROR_OUT_OF_DATE_KHR:
        return is_zh
                   ? "Surface 已变更且与交换链不兼容 (Out of Date)"
                   : "A surface has changed in such a way that it is no "
                   "longer compatible with the swapchain.";

    // Provided by VK_KHR_display_swapchain
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return is_zh
                   ? "交换链使用的显示设备不兼容"
                   : "The display used by a swapchain does not use the "
                   "same presentable image layout, or is incompatible.";

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
                   : "An external handle is not a valid handle of the "
                   "specified type.";

    // Provided by VK_VERSION_1_2
    case VK_ERROR_FRAGMENTATION:
        // Alias: VK_ERROR_FRAGMENTATION_EXT
        return is_zh
                   ? "由于碎片化，描述符池创建失败"
                   : "A descriptor pool creation has failed due to "
                   "fragmentation.";

    // Provided by VK_VERSION_1_2
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        // Alias: VK_ERROR_INVALID_DEVICE_ADDRESS_EXT
        // Alias: VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR
        return is_zh
                   ? "请求的地址不可用，导致缓冲创建或内存分配失败"
                   : "A buffer creation or memory allocation failed "
                   "because the requested address is not available.";

    // Provided by VK_EXT_full_screen_exclusive
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return is_zh
                   ? "独占全屏模式丢失"
                   : "An operation on a swapchain failed as it did not "
                   "have exclusive full-screen access.";

    // Provided by VK_VERSION_1_0 (Optimized out by default usually, but
    // valid return code)
    case VK_ERROR_VALIDATION_FAILED_EXT:
        // Note: VK_ERROR_VALIDATION_FAILED_EXT is an alias for value
        // -1000011001 Usually handled by layers, but defined in enum.
        return is_zh
                   ? "检测到无效用法 (Validation Failed)"
                   : "A command failed because invalid usage was "
                   "detected by the implementation or a validation "
                   "layer.";

    // Provided by VK_EXT_image_compression_control
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
        return is_zh
                   ? "图像创建失败，压缩所需内部资源耗尽"
                   : "An image creation failed because internal "
                   "resources required for compression are exhausted.";

    // Provided by VK_KHR_video_queue
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的 VkImageUsageFlags"
                   : "The requested VkImageUsageFlags are not supported.";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的视频图像布局"
                   : "The requested video picture layout is not "
                   "supported.";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持指定的视频配置操作"
                   : "A video profile operation specified via "
                   "VkVideoProfileInfoKHR::videoCodecOperation is not "
                   "supported.";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的视频配置格式参数"
                   : "Format parameters in a requested "
                   "VkVideoProfileInfoKHR chain are not supported.";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持请求的视频配置编解码器参数"
                   : "Codec-specific parameters in a requested "
                   "VkVideoProfileInfoKHR chain are not supported.";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
        return is_zh
                   ? "不支持指定的视频标准版本"
                   : "The specified video Std header version is not "
                   "supported.";

    // Provided by VK_KHR_video_encode_queue
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
        return is_zh
                   ? "视频标准参数无效或不符合语义要求"
                   : "The specified Video Std parameters do not adhere "
                   "to the syntactic or semantic requirements.";

    // Provided by VK_VERSION_1_4
    case VK_ERROR_NOT_PERMITTED:
        // Alias: VK_ERROR_NOT_PERMITTED_EXT, VK_ERROR_NOT_PERMITTED_KHR
        return is_zh
                   ? "权限不足，请求的高优先级被拒绝"
                   : "The driver implementation has denied a request to "
                   "acquire a priority above the default priority.";

    // Provided by VK_KHR_pipeline_binary
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
        return is_zh
                   ? "应用程序提供的空间不足以返回所有数据"
                   : "The application did not provide enough space to "
                   "return all the required data.";

    // Provided by VK_VERSION_1_0
    case VK_ERROR_UNKNOWN:
        return is_zh ? "发生未知错误" : "An unknown error has occurred.";

    default:
        return is_zh ? "未知的 VkResult 代码" : "Unknown VkResult code";
    }
}
} // namespace vulkan_utils
