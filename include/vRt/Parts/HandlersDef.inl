#pragma once
#include "Headers.inl"
#include "HardClassesDef.inl"

// class aliases for vRt from C++ hard implementators (incomplete)
// use shared pointers for C++
// (planned use plain pointers in C)
namespace vt { // store in official namespace

    struct VtInstance;
    struct VtPhysicalDevice;
    struct VtDevice;
    struct VtCommandBuffer;
    struct VtPipelineLayout;
    struct VtPipeline;
    struct VtAccelerator;
    struct VtMaterialSet;
    struct VtVertexInputSet;

    // advanced class (buffer)
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY>
    struct VtRoledBuffer;

    // advanced class (image)
    struct VtDeviceImage;

    // aliases
    using VtDeviceBuffer = VtRoledBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    using VtHostToDeviceBuffer = VtRoledBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    using VtDeviceToHostBuffer = VtRoledBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;

};