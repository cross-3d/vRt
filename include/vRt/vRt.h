#pragma once
#include "Parts/Headers.inl"
#include "Parts/StructuresLow.inl"
#include "Parts/StructuresDef.inl"
#include "Parts/HardClassesDef.inl"
#include "Parts/HandlersDef.inl"
#include "Parts/Enums.inl"
#include "Parts/HardClasses.inl"
#include "Parts/Handlers.inl"
#include "Parts/Structures.inl"
#include "Parts/API.inl"

// notify compilers or IDE about Vulkan API ray tracing (vRt) library connection
#define VRT_API_ENABLED
constexpr const static inline auto VRT_USE_MORTON_32 = false;

// implementators
#ifdef VRT_IMPLEMENTATION
#include "vRt_impl.inl"
#endif
