//
// Created by admin on 2026/2/11.
//

#pragma once
#include <vulkan/vulkan.hpp>

namespace vulkan_utils
{
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
constexpr std::string_view GetVkResultDescription(VkResult result, VkResultLang lang = VkResultLang::Chinese);
};

#include "vulkan_utils.ipp"
