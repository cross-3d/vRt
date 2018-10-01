#pragma once

#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION // implement VMA there
#endif

// Lowest sublevel of implementations for implementators
#include "../vRt.h"
#include "Utilities/VkUtils.hpp"

// implement these
#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
#include "Utilities/HardCode.hpp"
#else
#include "Utilities/SpvPaths.hpp"
#endif

#include "Definitions/Handlers.inl"
