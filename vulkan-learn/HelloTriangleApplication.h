//
// Created by zhang on 2025/11/1.
//

#ifndef VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H
#define VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H


#define GLFW_INCLUDE_VULKAN
#include <algorithm>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <magic_enum/magic_enum.hpp>

#include <format>
//#include <print>
#include <chrono>
#include <map>
#include <ranges>
#include <unordered_set>
#include <fmt/printf.h>


#include <glm/glm.hpp>
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

constexpr std::array<const char *, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef NODEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

template<typename T>
requires std::is_enum_v<T>
std::string GetFlagBitsDisplay(uint32_t Flags) {
    namespace rv = std::ranges::views;
    return magic_enum::enum_values<T>() | rv::filter([&](auto enum_value) {
        return enum_value & Flags;
    })| rv::transform([](auto enum_value) {return magic_enum::enum_name(enum_value);})
    | rv::join_with('|') | std::ranges::to<std::string>();
}


namespace vk_pred
{
    template<typename P1, typename P2>
    auto and_pred(P1 p1, P2 p2) {
        return [=](auto&&... args) {
            return p1(args...) && p2(args...);
        };
    }

    inline bool is_vk_queue_graphics(int index, const VkQueueFamilyProperties& queue_family_properties)
    {
        return queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
    }



}

namespace details {
    struct Any
    {
        template<typename T>
        operator T();
    };

    template<typename T, typename... A>
    constexpr auto arity()
    {
        if constexpr (requires { T{ Any{}, A{}... }; }) {
            return arity<T, Any, A...>();
        }
        else {
            return sizeof...(A);
        }
    };

    std::string get_project_dir();


    std::vector<char> readFile(const std::string& filePath);

}


template<typename T>
class VkBindingDescription;


struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

template<>
class VkBindingDescription<Vertex>{
public:
    constexpr static auto arity = details::arity<Vertex>();

    static consteval VkVertexInputBindingDescription getBindingDescription() {
        constexpr VkVertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        return bindingDescription;
    }

    static consteval std::array<VkVertexInputAttributeDescription, details::arity<Vertex>()> getAttributeDescriptions() {
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
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    static bool checkValidationLayerSupport(std::span<const char* const> layers);

    static bool checkDeviceExtensionsSupport(const VkPhysicalDevice device, const std::span<const char* const> extensions);

    void createLogicalDevice();

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    bool is_present_support(const int index,
                            const VkQueueFamilyProperties &queue_family_properties) const {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentSupport);
        return presentSupport == VK_TRUE;
    };
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


    std::vector<uint32_t> findQueueFamiliesIndex(VkPhysicalDevice device, std::function<bool(int, const VkQueueFamilyProperties&)> pred);

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };
    QueueFamilyIndices findQueueFamiliesIndex(VkPhysicalDevice device);

    int rateDeviceSuitability(VkPhysicalDevice device);

    void pickPhysicalDevice();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator);

    void setupDebugMessage();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    std::vector<const char *> getRequiredExtensions();


    void extensionCheck();

    void createInstance();

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        glfwSetErrorCallback([](int error_code, const char* description) {
            fmt::println("[glfw]error {}, {}", error_code, description);
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            const auto app = static_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
        });
    };

    void createVertexBuffer();;



    void createSurface() {
        if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void createSwapChain();

    void createImageView();


    void createGraphicsPipeline();
    void createComputePipeline();
    VkShaderModule createShaderModule(std::span<char> code);

    void createRenderPass();

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffer();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createSyncObject();

    void cleanupSwapChain();

    void recreateSwapChain();

    void initVulkan() {
        createInstance();
        setupDebugMessage();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageView();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffer();
        createSyncObject();
        recreateSwapChain();
        createVertexBuffer();

    }

    void drawFrame();

    void mainLoop() {
        auto tp = std::chrono::steady_clock::now();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(logicalDevice);
    }

    void cleanup();

    GLFWwindow *window = nullptr;
    VkInstance vkInstance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = nullptr;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue = nullptr;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool framebufferResized = false;
    uint32_t currentFlightFrame = 0;


    VkPipelineLayout compPipelineLayout;
    VkPipeline computePipeline;
    VkBuffer vertexBuffer;
};


#endif //VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H