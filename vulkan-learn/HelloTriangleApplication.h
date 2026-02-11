//
// Created by zhang on 2025/11/1.
//

#ifndef VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H
#define VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <array>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <glm/glm.hpp>

// ============ Constants ============

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 3;

constexpr std::array<const char*, 1> k_vulkan_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

inline const std::vector k_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NODEBUG
constexpr bool k_enable_validation_layers = false;
#else
constexpr bool k_enable_validation_layers = true;
#endif

// ============ Vertex definition ============

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static consteval vk::VertexInputBindingDescription get_binding_description()
    {
        return vk::VertexInputBindingDescription{
            0,
            sizeof(Vertex),
            vk::VertexInputRate::eVertex
        };
    }

    static consteval std::array<vk::VertexInputAttributeDescription, 2> get_attribute_descriptions()
    {
        return std::array<vk::VertexInputAttributeDescription, 2>{
            vk::VertexInputAttributeDescription{
                0,
                0,
                vk::Format::eR32G32Sfloat,
                offsetof(Vertex, pos),
            },
            vk::VertexInputAttributeDescription{
                1,
                0,
                vk::Format::eR32G32B32Sfloat,
                offsetof(Vertex, color),
            }
        };
    }
};

inline const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

inline const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

// ============ Utility declarations ============

namespace details
{
    std::string get_project_dir();
    std::vector<char> read_file(const std::string& filePath);
}

// ============ Application class ============

class HelloTriangleApplication
{
public:
    void run();

private:
    // --- Helper structs ---

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    // --- Stage 1: Window & Instance ---
    void init_window();
    void create_instance();
    void create_surface();
    void setup_debug_message();

    // --- Stage 2: Device selection ---
    void pick_physical_device();
    void create_logical_device();

    // --- Stage 3: Swap chain ---
    void create_swap_chain();
    void create_image_view();
    void cleanup_swap_chain() const;
    void recreate_swap_chain();

    // --- Stage 4: Render pipeline ---
    void create_render_pass();
    void create_graphics_pipeline();
    void create_framebuffers();

    // --- Stage 5: Commands & buffers ---
    void create_command_pool();
    void create_command_buffer();
    void create_vertex_buffer();
    void create_index_buffer();

    // --- Stage 6: Sync & rendering ---
    void create_sync_object();
    void record_command_buffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void draw_frame();

    // --- Lifecycle ---
    void init_vulkan();
    void main_loop();
    void cleanup();

    // --- Helper functions ---
    static bool check_instance_layer_support(std::span<const char* const> layers);
    static bool check_device_extensions_support(vk::PhysicalDevice device,
                                                const std::span<const char* const> extensions);
    static std::vector<const char*> get_required_extensions();
    static vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails query_swap_chain_support(vk::PhysicalDevice device);
    std::vector<uint32_t> find_queue_families_index(vk::PhysicalDevice device,
                                                    std::function<bool(int, const vk::QueueFamilyProperties&)> pred) const;
    QueueFamilyIndices find_queue_families_index(vk::PhysicalDevice device);
    bool is_present_support(int index, const vk::QueueFamilyProperties& queue_family_properties) const;
    int rate_device_suitability(vk::PhysicalDevice device);
    uint32_t find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                       vk::Buffer& buffer, vk::DeviceMemory& buffer_memory);
    void copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size);
    vk::ShaderModule create_shader_module(std::span<char> code);
    void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
    static VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // --- Member variables (grouped by stage) ---

    // Stage 1: Window & Instance
    GLFWwindow* window_ = nullptr;
    vk::Instance vk_instance_ = nullptr;
    vk::DebugUtilsMessengerEXT debug_messenger_{};
    vk::SurfaceKHR surface_{};

    // Stage 2: Devices
    vk::PhysicalDevice physical_device_{};
    vk::Device device_{};
    vk::Queue graphics_queue_{};
    vk::Queue present_queue_{};

    // Stage 3: Swap chain
    vk::SwapchainKHR swap_chain_{};
    std::vector<vk::Image> swap_chain_images_;
    vk::Format swap_chain_image_format_{};
    vk::Extent2D swap_chain_extent_{};
    std::vector<vk::ImageView> swap_chain_image_views_;

    // Stage 4: Pipeline
    vk::PipelineLayout pipeline_layout_{};
    vk::RenderPass render_pass_{};
    vk::Pipeline graphics_pipeline_{};
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    // Stage 5: Commands & buffers
    vk::CommandPool command_pool_{};
    std::vector<vk::CommandBuffer> command_buffers_;
    vk::Buffer vertex_buffer_{};
    vk::DeviceMemory vertex_buffer_memory_{};
    vk::Buffer index_buffer_{};
    vk::DeviceMemory index_buffer_memory_{};

    // Stage 6: Sync & frame state
    std::vector<vk::Semaphore> image_available_semaphores_;
    std::vector<vk::Semaphore> render_finished_semaphores_;
    std::vector<vk::Fence> fences_in_flight_;
    bool framebuffer_resized_ = false;
    uint32_t current_flight_frame_ = 0;
    uint32_t frame_count_ = 0;
};

#endif //VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H
