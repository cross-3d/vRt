#pragma once

// for correct VMA
#ifndef NOMINMAX
#define NOMINMAX
#endif

// unimplemented header 
#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION // require to implement VMA 
#endif
#include "vRt_internal.hpp"

// dedicated implementators 
#ifndef VRT_IMPLEMENTATION
#define VRT_IMPLEMENTATION
#include "Backland/vRt_impl.inl"
#endif
