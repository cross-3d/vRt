#pragma once

#include "Headers.inl"
//#include "StructuresDef.inl"
//#include "Enums.inl"

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

    // use roled buffers
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY> class RoledBuffer;

    using DeviceBuffer = RoledBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    using DeviceToHostBuffer = RoledBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;
    using HostToDeviceBuffer = RoledBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;

    // have no roles at now
    class DeviceImage;

    class BufferManager;
    class BufferRegion;
    class DeviceFeatures;
};