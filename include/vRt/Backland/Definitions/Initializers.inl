#pragma once

// implementable, used for describe types
#include "HardClassesDef.inl"
#include "Structures.inl" // required for default arguments, but in general - useless in here

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vrt;

    // base creation functions 
    extern VtResult convertInstance(VkInstance vkInstance, const VtInstanceConversionInfo& vtInstanceCreateInfo, std::shared_ptr<Instance>& vtInstance);
    extern VtResult convertPhysicalDevice(const std::shared_ptr<Instance>& instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& _vtPhysicalDevice);
    extern VtResult convertDevice(VkDevice device, const std::shared_ptr<PhysicalDevice>& physicalDevice, const VtDeviceAggregationInfo& vtExtension, std::shared_ptr<Device>& vtDevice);

    template<VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY>
    extern VtResult createBuffer(const std::shared_ptr<Device>& device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>>& _vtBuffer);

    // artifical function type
    template<VtMemoryUsage U>
    using _createBuffer_T = VtResult(*)(const std::shared_ptr<Device>& device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>> &_vtBuffer);

    // aliased calls
    constexpr const static inline _createBuffer_T<VT_MEMORY_USAGE_GPU_ONLY> createDeviceBuffer = &createBuffer<VT_MEMORY_USAGE_GPU_ONLY>;
    constexpr const static inline _createBuffer_T<VT_MEMORY_USAGE_GPU_TO_CPU> createHostToDeviceBuffer = &createBuffer<VT_MEMORY_USAGE_GPU_TO_CPU>;
    constexpr const static inline _createBuffer_T<VT_MEMORY_USAGE_CPU_TO_GPU> createDeviceToHostBuffer = &createBuffer<VT_MEMORY_USAGE_CPU_TO_GPU>;
    extern VtResult createDeviceImage(const std::shared_ptr<Device>& device, const VtDeviceImageCreateInfo& cinfo, std::shared_ptr<DeviceImage>& _vtImage);
    extern VtResult createSharedBuffer(const std::shared_ptr<BufferManager>& bManager, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<DeviceBuffer>& gBuffer);
    extern VtResult createSharedBuffer(const std::shared_ptr<BufferManager>& bManager, const VtDeviceBufferCreateInfo& cinfo);
    extern VtResult createBufferManager(const std::shared_ptr<Device>& vtDevice, std::shared_ptr<BufferManager>& bManager);
    extern VtResult createBufferRegion(const std::shared_ptr<DeviceBuffer>& gBuffer, const VtBufferRegionCreateInfo& bri, std::shared_ptr<BufferRegion>& bRegion);
    extern VtResult createBufferRegion(const std::shared_ptr<BufferManager>& bManager, const VtBufferRegionCreateInfo& bri, std::shared_ptr<BufferRegion>& bRegion);
    extern VtResult createBufferRegion(VkBuffer vkBuffer, const VtBufferRegionCreateInfo& bri, std::shared_ptr<BufferRegion>& bRegion, const std::shared_ptr<Device>& vtDevice = {});

    // main inner objects
    extern VtResult createDevice(const std::shared_ptr<PhysicalDevice>& physicalDevice, const VkDeviceCreateInfo& vdvi, std::shared_ptr<Device>& vtDevice);
    extern VtResult createAcceleratorHLBVH2(const std::shared_ptr<Device>& vtDevice, const VtDeviceAggregationInfo& info, std::shared_ptr<AcceleratorHLBVH2>& _vtAccelerator);
    extern VtResult createAcceleratorSet(const std::shared_ptr<Device>& vtDevice, const VtAcceleratorSetCreateInfo& info, std::shared_ptr<AcceleratorSet>& _vtAccelerator);
    extern VtResult createRadixSort(const std::shared_ptr<Device>& vtDevice, const VtDeviceAggregationInfo& info, std::shared_ptr<RadixSort>& _vtRadix);
    extern VtResult createAssemblyPipeline(const std::shared_ptr<Device>& vtDevice, const VtAttributePipelineCreateInfo& info, std::shared_ptr<AssemblyPipeline>& _assemblyPipeline, bool native = false);
    extern VtResult createVertexAssemblySet(const std::shared_ptr<Device>& vtDevice, const VtVertexAssemblySetCreateInfo& info, std::shared_ptr<VertexAssemblySet>& _assemblyPipeline);
    extern VtResult createVertexInputSet(const std::shared_ptr<Device>& vtDevice, const VtVertexInputCreateInfo& info, std::shared_ptr<VertexInputSet>& _vtVertexInput);
    extern VtResult createPipelineLayout(const std::shared_ptr<Device>& vtDevice, const VtPipelineLayoutCreateInfo& vtPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout, const VtPipelineLayoutType& type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING);
    extern VtResult createMaterialSet(const std::shared_ptr<Device>& vtDevice, const VtMaterialSetCreateInfo& info, std::shared_ptr<MaterialSet>& _vtMaterialSet);
    extern VtResult createRayTracingPipeline(const std::shared_ptr<Device>& vtDevice, const VtRayTracingPipelineCreateInfo& info, std::shared_ptr<Pipeline>& _vtPipeline);
    extern VtResult createRayTracingSet(const std::shared_ptr<Device>& vtDevice, const VtRayTracingSetCreateInfo& info, std::shared_ptr<RayTracingSet>& _vtRTSet);

    // create wrapped command buffer
    extern VtResult queryCommandBuffer(const std::shared_ptr<Device>& vtDevice, VkCommandBuffer cmdBuf, std::shared_ptr<CommandBuffer>& vtCmdBuf);
};
