#pragma once

#include "StructuresDef.inl"

// store in official namespace
namespace vrt {
    class VtInstance;
    class VtPhysicalDevice;
    class VtDevice;
    class VtCommandBuffer;
    class VtPipelineLayout;
    class VtPipeline;
    class VtAccelerator;
    class VtMaterialSet;
    class VtVertexInputSet;
    class VtAttributePipeline;
    class VtVertexAssemblySet;
    class VtAcceleratorSet;
    class VtRayTracingSet;

    // advanced class (buffer)
    //class VtRoledBufferBase; // 
    template<VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY>
    class VtRoledBuffer;

    // advanced class (image)
    class VtDeviceImage;

    // aliases
    using VtDeviceBuffer = VtRoledBuffer< VT_MEMORY_USAGE_GPU_ONLY >;
    using VtHostToDeviceBuffer = VtRoledBuffer< VT_MEMORY_USAGE_CPU_TO_GPU >;
    using VtDeviceToHostBuffer = VtRoledBuffer< VT_MEMORY_USAGE_GPU_TO_CPU >;
};
