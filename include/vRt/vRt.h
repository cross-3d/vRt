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

// linux not support indirect implementation
//#ifndef RVT_IMPLEMENTATION
//#define RVT_IMPLEMENTATION
//#endif

// notify compilers or IDE about Vulkan API ray tracing (vRt) library connection
#define RVT_API_ENABLED
//#define RVT_USE_MORTON_32 true
constexpr auto RVT_USE_MORTON_32 = true;

// implementators
#ifdef RVT_IMPLEMENTATION
#include "vRt_impl.inl"
#endif
