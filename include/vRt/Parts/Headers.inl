#pragma once

// include all related with "Vulkan API" for headers 
#include <stdio.h>

#ifndef VULKAN_H_
#include <vulkan/volk.h>
#endif

#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
#include <vulkan/vk_mem_alloc.h>
#endif

#include <map>
#include <memory>

#ifdef ENABLE_VRT_STRING_VIEW
#include <string_view>
#endif

#include <vector>
#include <functional>
