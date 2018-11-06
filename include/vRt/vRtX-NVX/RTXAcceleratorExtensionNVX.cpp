#ifndef NOMINMAX
#define NOMINMAX
#endif

// static library of RTX support (WIP)
#define VK_NO_PROTOTYPES
#define VT_LEGACY_RAYTRACING_NVX

#include "vulkan/vulkan.h"
#include "vulkan/volk.h"
#include "vulkan/vulkan.hpp"
#include "../Backland/vRt_subimpl.inl"
#include "RTXImplemented.inl"
