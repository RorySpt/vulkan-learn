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
    inline bool is_vk_queue_graphics(const VkQueueFamilyProperties& queue_family_properties)
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

    static consteval std::array<VkVertexInputAttributeDescription, details::arity<Vertex>()> getAttributeDescription() {
        constexpr std::array<VkVertexInputAttributeDescription, details::arity<Vertex>()> attributeDescription{
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
        return attributeDescription;
    }
};


const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};


class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void createLogicalDevice();

    std::vector<uint32_t> findQueueFamiliesIndex(VkPhysicalDevice device, std::function<bool(const VkQueueFamilyProperties&)> pred);

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

    bool checkValidationLayerSupport();

    void extensionCheck();

    void createInstance();

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void createVertexBuffer() {
        auto bindingDescription = VkBindingDescription<Vertex>::getBindingDescription();
        auto attributeDescription = VkBindingDescription<Vertex>::getAttributeDescription();

        VkBufferCreateInfo vkBufferCreateInfo{};
        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkBufferCreateInfo.size = sizeof(Vertex);
    };

    void initVulkan() {
        createInstance();
        setupDebugMessage();
        pickPhysicalDevice();
        createLogicalDevice();
        createVertexBuffer();

    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();



        }
    }

    void cleanup() {
        vkDestroyDevice(logicalDevice, nullptr);
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
        }
        vkDestroyInstance(vkInstance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow *window = nullptr;
    VkInstance vkInstance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = nullptr;
    VkQueue graphicsQueue = nullptr;
};


#endif //VULKAN_LEARN_HELLOTRIANGLEAPPLICATION_H