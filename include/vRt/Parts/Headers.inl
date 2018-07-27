#pragma once

// include all related with "Vulkan API" for headers 
#include <stdio.h>
//#ifdef RVT_USE_VOLK
#ifndef VULKAN_H_
#include <vulkan/volk.h>
#endif
//#else
//#include <vulkan/vulkan.h>
//#endif
#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
#include <vulkan/vk_mem_alloc.h>
#endif

#include <map>
#include <memory>
#include <string_view>
#include <vector>
#include <functional>
