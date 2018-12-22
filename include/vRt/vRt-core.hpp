#pragma once

// use basic Vulkan API
#include "vulkan/volk.h"

// use some STL stuff
#include <stdio.h>
#include <vector>
#include <memory>

// mainline namespace (with API)
namespace vrt {

};

// if required implementation here 
#ifdef VRT_IMPLEMENTATION
#include "vRt-core.inl"
#endif
