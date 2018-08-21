#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vrt;

    // base creation functions 
    extern VtResult convertInstance(VkInstance vkInstance, const VtInstanceConversionInfo& vtInstanceCreateInfo, std::shared_ptr<Instance>& vtInstance);
    extern VtResult convertPhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& _vtPhysicalDevice);
    extern VtResult convertDevice(VkDevice device, std::shared_ptr<PhysicalDevice> physicalDevice, const VtArtificalDeviceExtension& vtExtension, std::shared_ptr<Device>& _vtDevice);

    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY>
    extern VtResult createBuffer(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>>& _vtBuffer);

    // artifical function type
    template<VmaMemoryUsage U>
    using _createBuffer_T = VtResult(*)(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>> &_vtBuffer);

    // aliased calls
    constexpr const static inline _createBuffer_T<VMA_MEMORY_USAGE_GPU_ONLY> createDeviceBuffer = &createBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr const static inline _createBuffer_T<VMA_MEMORY_USAGE_CPU_TO_GPU> createHostToDeviceBuffer = &createBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr const static inline _createBuffer_T<VMA_MEMORY_USAGE_GPU_TO_CPU> createDeviceToHostBuffer = &createBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;
    extern VtResult createDeviceImage(std::shared_ptr<Device> device, const VtDeviceImageCreateInfo& cinfo, std::shared_ptr<DeviceImage>& _vtImage);
    extern VtResult createSharedBuffer(const std::shared_ptr<BufferManager>& bManager, VtDeviceBufferCreateInfo cinfo);

    // main inner objects
    extern VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo vdvi, std::shared_ptr<Device>& _vtDevice);
    extern VtResult createAcceleratorHLBVH2(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<AcceleratorHLBVH2>& _vtAccelerator);
    extern VtResult createAcceleratorSet(std::shared_ptr<Device> _vtDevice, const VtAcceleratorSetCreateInfo &info, std::shared_ptr<AcceleratorSet>& _vtAccelerator);
    extern VtResult createRadixSort(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<RadixSort>& _vtRadix);
    extern VtResult createVertexAssemblyPipeline(std::shared_ptr<Device> _vtDevice, const VtVertexAssemblyPipelineCreateInfo& info, std::shared_ptr<VertexAssemblyPipeline>& _vtVertexAssembly);
    extern VtResult createVertexAssemblySet(std::shared_ptr<Device> _vtDevice, const VtVertexAssemblySetCreateInfo &info, std::shared_ptr<VertexAssemblySet>& _vtVertexAssembly);
    extern VtResult createVertexInputSet(std::shared_ptr<Device> _vtDevice, VtVertexInputCreateInfo info, std::shared_ptr<VertexInputSet>& _vtVertexInput);
    extern VtResult createPipelineLayout(std::shared_ptr<Device> _vtDevice, VtPipelineLayoutCreateInfo vtPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout, VtPipelineLayoutType type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING);
    extern VtResult createMaterialSet(std::shared_ptr<Device> _vtDevice, const VtMaterialSetCreateInfo& info, std::shared_ptr<MaterialSet>& _vtMaterialSet);
    extern VtResult createRayTracingPipeline(std::shared_ptr<Device> _vtDevice, const VtRayTracingPipelineCreateInfo& info, std::shared_ptr<Pipeline>& _vtPipeline);
    extern VtResult createRayTracingSet(std::shared_ptr<Device> _vtDevice, const VtRayTracingSetCreateInfo& info, std::shared_ptr<RayTracingSet>& _vtRTSet);

    // create wrapped command buffer
    inline VtResult queryCommandBuffer(std::shared_ptr<Device> _vtDevice, VkCommandBuffer cmdBuf, std::shared_ptr<CommandBuffer>& vtCmdBuf) {
        vtCmdBuf = std::make_shared<CommandBuffer>();
        vtCmdBuf->_device = _vtDevice;
        vtCmdBuf->_commandBuffer = cmdBuf;
        return VK_SUCCESS;
    };

};
