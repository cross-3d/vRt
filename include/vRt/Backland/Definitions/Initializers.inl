#pragma once

// implementable, used for describe types
#include "HardClassesDef.inl"
#include "Structures.inl" // required for default arguments, but in general - useless in here

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vrt;

    // base creation functions 
    extern VtResult convertInstance(VkInstance vkInstance, VtInstanceConversionInfo vtInstanceCreateInfo, std::shared_ptr<Instance>& vtInstance);
    extern VtResult convertPhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& _vtPhysicalDevice);
    extern VtResult convertDevice(VkDevice device, std::shared_ptr<PhysicalDevice> physicalDevice, VtDeviceAggregationInfo vtExtension, std::shared_ptr<Device>& _vtDevice);

    template<VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY>
    extern VtResult createBuffer(std::shared_ptr<Device> device, VtDeviceBufferCreateInfo cinfo, std::shared_ptr<RoledBuffer<U>>& _vtBuffer);

    // artifical function type
    template<VtMemoryUsage U>
    using _createBuffer_T = VtResult(*)(std::shared_ptr<Device> device, VtDeviceBufferCreateInfo cinfo, std::shared_ptr<RoledBuffer<U>> &_vtBuffer);

    // aliased calls
    constexpr const static inline _createBuffer_T<VT_MEMORY_USAGE_GPU_ONLY> createDeviceBuffer = &createBuffer<VT_MEMORY_USAGE_GPU_ONLY>;
    constexpr const static inline _createBuffer_T<VT_MEMORY_USAGE_CPU_TO_GPU> createHostToDeviceBuffer = &createBuffer<VT_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr const static inline _createBuffer_T<VT_MEMORY_USAGE_GPU_TO_CPU> createDeviceToHostBuffer = &createBuffer<VT_MEMORY_USAGE_GPU_TO_CPU>;
    extern VtResult createDeviceImage(std::shared_ptr<Device> device, VtDeviceImageCreateInfo cinfo, std::shared_ptr<DeviceImage>& _vtImage);
    extern VtResult createSharedBuffer(std::shared_ptr<BufferManager> bManager, VtDeviceBufferCreateInfo cinfo, std::shared_ptr<DeviceBuffer>& gBuffer);
    extern VtResult createSharedBuffer(std::shared_ptr<BufferManager> bManager, VtDeviceBufferCreateInfo cinfo);
    extern VtResult createBufferManager(std::shared_ptr<Device> gDevice, std::shared_ptr<BufferManager>& bManager);
    extern VtResult createBufferRegion(std::shared_ptr<DeviceBuffer> gBuffer, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion);
    extern VtResult createBufferRegion(std::shared_ptr<BufferManager> bManager, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion);
    extern VtResult createBufferRegion(VkBuffer vkBuffer, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion, std::shared_ptr<Device> gDevice = {});

    // main inner objects
    extern VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo vdvi, std::shared_ptr<Device>& _vtDevice);
    extern VtResult createAcceleratorHLBVH2(std::shared_ptr<Device> _vtDevice, VtDeviceAggregationInfo info, std::shared_ptr<AcceleratorHLBVH2>& _vtAccelerator);
    extern VtResult createAcceleratorSet(std::shared_ptr<Device> _vtDevice, VtAcceleratorSetCreateInfo info, std::shared_ptr<AcceleratorSet>& _vtAccelerator);
    extern VtResult createRadixSort(std::shared_ptr<Device> _vtDevice, VtDeviceAggregationInfo info, std::shared_ptr<RadixSort>& _vtRadix);
    extern VtResult createAssemblyPipeline(std::shared_ptr<Device> _vtDevice, VtAttributePipelineCreateInfo info, std::shared_ptr<AssemblyPipeline>& _assemblyPipeline, bool native = false);
    extern VtResult createVertexAssemblySet(std::shared_ptr<Device> _vtDevice, VtVertexAssemblySetCreateInfo info, std::shared_ptr<VertexAssemblySet>& _assemblyPipeline);
    extern VtResult createVertexInputSet(std::shared_ptr<Device> _vtDevice, VtVertexInputCreateInfo info, std::shared_ptr<VertexInputSet>& _vtVertexInput);
    extern VtResult createPipelineLayout(std::shared_ptr<Device> _vtDevice, VtPipelineLayoutCreateInfo vtPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout, const VtPipelineLayoutType& type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING);
    extern VtResult createMaterialSet(std::shared_ptr<Device> _vtDevice, VtMaterialSetCreateInfo info, std::shared_ptr<MaterialSet>& _vtMaterialSet);
    extern VtResult createRayTracingPipeline(std::shared_ptr<Device> _vtDevice, VtRayTracingPipelineCreateInfo info, std::shared_ptr<Pipeline>& _vtPipeline);
    extern VtResult createRayTracingSet(std::shared_ptr<Device> _vtDevice, VtRayTracingSetCreateInfo info, std::shared_ptr<RayTracingSet>& _vtRTSet);

    // create wrapped command buffer
    extern VtResult queryCommandBuffer(std::shared_ptr<Device> _vtDevice, VkCommandBuffer cmdBuf, std::shared_ptr<CommandBuffer>& vtCmdBuf);
};
