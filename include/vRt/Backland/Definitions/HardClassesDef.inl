#pragma once

// implementable
#include "../../../vRt/vRt_internal.hpp"
//

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
    //class AcceleratorLinkedSet;
    class AssemblyPipeline;
    class VertexAssemblySet;
    class RayTracingSet;
    class BufferTraffic;
    class PipelineLayout;

    // use roled buffers
    // TODO: need's unified base class for some cases
    class RoledBufferBase;
    template<VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY> class RoledBuffer;

    using DeviceBuffer = RoledBuffer<VT_MEMORY_USAGE_GPU_ONLY>;
    using DeviceToHostBuffer = RoledBuffer<VT_MEMORY_USAGE_GPU_TO_CPU>;
    using HostToDeviceBuffer = RoledBuffer<VT_MEMORY_USAGE_CPU_TO_GPU>;

    // have no roles at now
    class DeviceImage;

    class BufferManager;
    class BufferRegion;
    class DeviceFeatures;

    // will inherits
    class AcceleratorExtensionBase;

    // extension for accelerator sets
    class AcceleratorSetExtensionBase;

    // extension for vertex assembly
    class VertexAssemblyExtensionBase;
};
