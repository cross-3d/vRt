#pragma once
#include "Headers.inl"
#include "HardClassesDef.inl"

// store in official namespace
namespace vrt {
    struct VtInstance;
    struct VtPhysicalDevice;
    struct VtDevice;
    struct VtCommandBuffer;
    struct VtPipelineLayout;
    struct VtPipeline;
    struct VtAccelerator;
    struct VtMaterialSet;
    struct VtVertexInputSet;
    struct VtVertexAssemblyPipeline;

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
