#pragma once

// if vulkan header included, make compatible with volk.h
#ifdef VULKAN_H_
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
#ifndef VEZ_H
#include <VEZ/VEZ.h>
#endif
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

