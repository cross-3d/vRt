#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vt;

    inline VtResult makePhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& _vtPhysicalDevice);

    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY>
    inline VtResult createBuffer(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>>& _vtBuffer);

    // artifical function type
    template<VmaMemoryUsage U>
    using _createBuffer_T = VtResult(*)(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>> &_vtBuffer);

    // aliased calls
    constexpr _createBuffer_T<VMA_MEMORY_USAGE_GPU_ONLY> createDeviceBuffer = &createBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr _createBuffer_T<VMA_MEMORY_USAGE_CPU_TO_GPU> createHostToDeviceBuffer = &createBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr _createBuffer_T<VMA_MEMORY_USAGE_GPU_TO_CPU> createDeviceToHostBuffer = &createBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;
    inline VtResult createDeviceImage(std::shared_ptr<Device> device, const VtDeviceImageCreateInfo& cinfo, std::shared_ptr<DeviceImage>& _vtImage);

    // main inner objects
    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo vdvi, std::shared_ptr<Device>& _vtDevice);
    inline VtResult createAccelerator(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<Accelerator>& _vtAccelerator);
    inline VtResult createAcceleratorSet(std::shared_ptr<Device> _vtDevice, const VtAcceleratorSetCreateInfo &info, std::shared_ptr<AcceleratorSet>& _vtAccelerator);
    inline VtResult createRadixSort(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<RadixSort>& _vtRadix);
    inline VtResult createVertexAssembly(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<VertexAssembly>& _vtVertexAssembly);
    inline VtResult createVertexAssemblySet(std::shared_ptr<Device> _vtDevice, const VtAcceleratorSetCreateInfo &info, std::shared_ptr<VertexAssemblySet>& _vtVertexAssembly);
    inline VtResult createVertexInputSet(std::shared_ptr<Device> _vtDevice, VtVertexInputCreateInfo& info, std::shared_ptr<VertexInputSet>& _vtVertexInput);
    inline VtResult createRayTracingPipelineLayout(std::shared_ptr<Device> _vtDevice, VkPipelineLayoutCreateInfo vtRayTracingPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout);

};
