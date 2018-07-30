#pragma once

/*
// compatible with API 
#ifdef VK_VERSION_1_0
#undef VK_VERSION_1_0
#endif

// compatible with API 1.1
#ifdef VK_VERSION_1_1
#undef VK_VERSION_1_1
#endif
*/

// use Vulkan API 1.1
//#ifndef VK_VERSION_1_1
//#define VK_VERSION_1_1
//#endif

// if vulkan header included, make compatible with volk.h
#if (defined(VULKAN_H_) && !defined(VRT_DONT_USE_VOLK))
#define VK_NO_PROTOTYPES
#endif

// include volk.h when possible
#ifndef VULKAN_H_
#ifdef VRT_DONT_USE_VOLK
#include <vulkan/vulkan.h>
#else
#include <vulkan/volk.h>
#endif
#endif

// include VEZ for interop
#ifdef VRT_ENABLE_VEZ_INTEROP
#include <VEZ/VEZ.h>
#endif

// if no defined VEZ, and not included VMA, include it
//#if (!defined(AMD_VULKAN_MEMORY_ALLOCATOR_H) && !defined(VRT_ENABLE_VEZ_INTEROP))
#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
#include <vulkan/vk_mem_alloc.h>
#endif

// include C++17 capable STL
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <cstdint>
#include <cstddef>

// include late C++17 string_view support
#ifdef VRT_ENABLE_STRING_VIEW
#include <string_view>
#endif

