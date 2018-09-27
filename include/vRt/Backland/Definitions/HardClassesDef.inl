#pragma once

// implementable
#include "../../Parts/Headers.inl"

// C++ hard interfaces (which will storing)
namespace _vt { // store in undercover namespace
    using namespace vrt;

    template<class T> class VtHandle;

    class Instance;
    class PhysicalDevice;
    class Device;
    class RadixSort;
    class CommandBuffer;
    class Pipeline;
    class CopyProgram;
    class MaterialSet;
    class VertexInputSet;
    class AcceleratorHLBVH2;
    class AcceleratorSet;
    class VertexAssemblyPipeline;
    class VertexAssemblySet;
    class RayTracingSet;
    class BufferTraffic;
    class PipelineLayout;

    // use roled buffers
    // TODO: need's unified base class for some cases
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY> class RoledBuffer;

    using DeviceBuffer = RoledBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    using DeviceToHostBuffer = RoledBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;
    using HostToDeviceBuffer = RoledBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;

    // have no roles at now
    class DeviceImage;

    class BufferManager;
    class BufferRegion;
    class DeviceFeatures;

    // will inherits
    class AdvancedAcceleratorBase;
    class AdvancedAcceleratorDataBase;

    // extension for accelerator sets
    class AcceleratorSetExtensionBase;
    class AcceleratorSetExtensionDataBase;
};
