//
// Created by zhang on 2025/11/1.
//

#ifndef VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H
#define VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H


#define GLFW_INCLUDE_VULKAN
#include <algorithm>
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <magic_enum/magic_enum.hpp>

#include <format>
//#include <print>
#include <chrono>
#include <map>
#include <ranges>
#include <range/v3/all.hpp>
#include <unordered_set>
#include <fmt/printf.h>


#include <glm/glm.hpp>

#include "HelloTriangleApplication.h"
#include "HelloTriangleApplication.h"
#include "HelloTriangleApplication.h"
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

constexpr std::array<const char*, 1> k_vulkan_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector k_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef NODEBUG
constexpr bool k_enable_validation_layers = false;
#else
constexpr bool k_enable_validation_layers = true;
#endif

template <typename T>
    requires std::is_enum_v<T>
std::string convert_flags_to_names(uint32_t Flags)
{
    namespace rv = std::ranges::views;
    return magic_enum::enum_values<T>()
        | rv::filter([&](auto enum_value) { return static_cast<uint32_t>(enum_value) & Flags; })
        | rv::transform([](auto enum_value) { return magic_enum::enum_name(enum_value); })
        | rv::join_with('|')
        | std::ranges::to<std::string>();
}

template <typename T>
    requires std::is_enum_v<T>
std::string convert_flags_to_names(vk::Flags<T> Flags)
{
    namespace rv = std::ranges::views;
    return magic_enum::enum_values<T>()
        | rv::filter([Flags](T enum_value) { return !!(enum_value & Flags); })
        | rv::transform([](T enum_value) { return magic_enum::enum_name(enum_value); })
        | rv::join_with('|')
        | std::ranges::to<std::string>();
}


namespace vk_pred
{
    template <typename P1, typename P2>
    auto and_pred(P1 p1, P2 p2)
    {
        return [=](auto&&... args)
        {
            return p1(args...) && p2(args...);
        };
    }

    inline bool is_vk_queue_graphics(int index, const VkQueueFamilyProperties& queue_family_properties)
    {
        return queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
    }
}

namespace details
{
    struct Any
    {
        template <typename T>
        operator T();
    };

    template <typename T, typename... A>
    constexpr auto arity()
    {
        if constexpr (requires { T{Any{}, A{}...}; })
        {
            return arity<T, Any, A...>();
        }
        else
        {
            return sizeof...(A);
        }
    };

    std::string get_project_dir();


    std::vector<char> read_file(const std::string& filePath);
}


template <typename T>
class VkBindingDescription;


struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

template <>
class VkBindingDescription<Vertex>
{
public:
    constexpr static auto arity = details::arity<Vertex>();

    static consteval VkVertexInputBindingDescription get_binding_description()
    {
        constexpr VkVertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        return bindingDescription;
    }

    static consteval std::array<VkVertexInputAttributeDescription, details::arity<Vertex>()> get_attribute_descriptions()
    {
        constexpr std::array<VkVertexInputAttributeDescription, details::arity<Vertex>()> attributeDescriptions{
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, pos),
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            }
        };
        return attributeDescriptions;
    }
};


const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

class HelloTriangleApplication
{
public:
    void run()
    {
        init_window();
        init_vulkan();
        main_loop();
        cleanup();
    }

private:
    static bool check_validation_layer_support(std::span<const char* const> layers);

    static bool check_device_extensions_support(VkPhysicalDevice device,
                                                const std::span<const char* const> extensions);

    void create_logical_device();

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    bool is_present_support(const int index,
                            const VkQueueFamilyProperties& queue_family_properties) const
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, index, surface_, &presentSupport);
        return presentSupport == VK_TRUE;
    };
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);


    std::vector<uint32_t> find_queue_families_index(vk::PhysicalDevice device,
                                                    std::function<bool(int, const VkQueueFamilyProperties&)> pred) const;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    QueueFamilyIndices find_queue_families_index(VkPhysicalDevice device);

    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    int rate_device_suitability(VkPhysicalDevice device);

    void pick_physical_device();

    void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    static VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkDebugUtilsMessengerEXT* pDebugMessenger);

    void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                           const VkAllocationCallbacks* pAllocator);

    void setup_debug_message();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    std::vector<const char*> get_required_extensions();


    void extension_check();

    void create_instance();

    static VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    static VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

    void init_window()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);

        glfwSetErrorCallback([](int error_code, const char* description)
        {
            fmt::println("[glfw]error {}, {}", error_code, description);
        });
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height)
        {
            const auto app = static_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
            app->framebuffer_resized_ = true;
        });
    };

    void create_vertex_buffer();
    void create_index_buffer();

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                       VkDeviceMemory& buffer_memory);

    void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

    void create_surface()
    {
        if (glfwCreateWindowSurface(vk_instance_, window_, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface_)) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void create_swap_chain();

    void create_image_view();


    void create_graphics_pipeline();

    VkShaderModule create_shader_module(std::span<char> code);

    void create_render_pass();

    void create_framebuffers();

    void create_command_pool();

    void create_command_buffer();

    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void create_sync_object();

    void cleanup_swap_chain() const;

    void recreate_swap_chain();

    void init_vulkan()
    {
        create_instance();
        // setup_debug_message(); //setup at create_instance
        create_surface();
        pick_physical_device();
        create_logical_device();
        create_swap_chain();
        create_image_view();
        create_render_pass();
        create_graphics_pipeline();
        create_framebuffers();
        create_command_pool();
        create_vertex_buffer();
        create_command_buffer();
        create_sync_object();
        recreate_swap_chain();

        std::cout << std::flush;
        std::cerr << std::flush;
    }

    void draw_frame();

    void main_loop()
    {
        auto tp = std::chrono::steady_clock::now();
        while (!glfwWindowShouldClose(window_))
        {
            glfwPollEvents();
            draw_frame();
            auto last_tp = std::exchange(tp, std::chrono::steady_clock::now());
            auto cost_ms = std::chrono::duration<double>(tp - last_tp).count() * 1000;

            //fmt::println("frame cost: {: .4f}ms, fps: {}", cost_ms, static_cast<uint64_t>(1000 / cost_ms));
        }
        vkDeviceWaitIdle(device_);
    }

    void cleanup();

    GLFWwindow* window_ = nullptr;
    vk::Instance vk_instance_ = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger_ = nullptr;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = nullptr;
    vk::SurfaceKHR surface_{};
    VkQueue graphics_queue_ = nullptr;
    VkQueue present_queue_{};
    VkSwapchainKHR swap_chain_{};
    std::vector<VkImage> swap_chain_images_;
    VkFormat swap_chain_image_format_{};
    VkExtent2D swap_chain_extent_{};
    std::vector<VkImageView> swap_chain_image_views_;
    VkPipelineLayout pipeline_layout_{};
    VkRenderPass render_pass_{};
    VkPipeline graphics_pipeline_{};
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool command_pool_{};

    std::vector<VkCommandBuffer> command_buffers_;
    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> fences_in_flight_;
    std::map<uint32_t, uint32_t> image_available_semaphores_map_;
    std::map<uint32_t, uint32_t> available_semaphores_image_map_;
    bool framebuffer_resized_ = false;
    uint32_t current_flight_frame_ = 0;
    uint32_t frame_count_ = 0;
    VkBuffer vertex_buffer_{};
    VkDeviceMemory vertex_buffer_memory_{};
    VkBuffer index_buffer_{};
    VkDeviceMemory index_buffer_memory_{};
};


#endif //VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H
