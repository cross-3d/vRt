#pragma once

#include "../../vRt_subimpl.inl"
#include "./BuildAccelerator.inl"
#include "./RayTracing.inl"
#include "./RadixSort.inl"

namespace vt {
    using namespace _vt;

    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice) {
        return createDevice(vtPhysicalDevice, *deviceCreateInfo, vtDevice->_vtDevice);
    };

    inline VtResult vtConvertDevice(VtPhysicalDevice vtPhysicalDevice, VkDevice vkDevice, const VtArtificalDeviceExtension * vtDeviceExtension, VtDevice * vtDevice) {
        return convertDevice(vkDevice, vtPhysicalDevice, *vtDeviceExtension, *vtDevice);
    };

    inline VtResult vtCmdImageBarrier(VkCommandBuffer cmd, VtDeviceImage image) {
        return imageBarrier(cmd, image);
    };

    inline VtResult vtCreateRayTracingPipelineLayout(VtDevice device, const VkPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout) {
        return createRayTracingPipelineLayout(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipelineLayout);
    };

    inline VtResult vtCreateRayTracingPipeline(VtDevice device, const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipeline * vtPipeline) {
        return createRayTracingPipeline(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipeline);
    };

    inline VtResult vtCreateRayTracingSet(VtDevice device, const VtRayTracingSetCreateInfo * vtSetCreateInfo, VtRayTracingSet * vtSet) {
        return createRayTracingSet(device, *vtSetCreateInfo, *vtSet);
    };

    inline VtResult vtCreateAccelerator(VtDevice device, const VtAcceleratorSetCreateInfo * vtAcceleratorCreateInfo, VtAcceleratorSet * accelerator) {
        return createAcceleratorSet(device, *vtAcceleratorCreateInfo, *accelerator);
    };

    inline VtResult vtCreateVertexAssembly(VtDevice device, const VtVertexAssemblySetCreateInfo * vtVertexAssemblyCreateInfo, VtVertexAssemblySet * vertexAssembly) {
        return createVertexAssemblySet(device, *vtVertexAssemblyCreateInfo, *vertexAssembly);
    };

    inline VtResult vtCreateMaterialSet(VtDevice device, const VtMaterialSetCreateInfo * vtMaterialsCreateInfo, VtMaterialSet * materialsInput) {
        return createMaterialSet(device, *vtMaterialsCreateInfo, *materialsInput);
    };

    inline VtResult vtCreateVertexInputSet(VtDevice device, const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInputSet * vertexInputSet) {
        return createVertexInputSet(device, *vtVertexInputCreateInfo, *vertexInputSet);
    };

    inline VtResult vtQueryCommandInterface(VtDevice device, VkCommandBuffer commandBuffer, VtCommandBuffer * vtCommandBuffer) {
        return queryCommandBuffer(device, commandBuffer, *vtCommandBuffer);
    };

    inline VtResult vtCmdBindPipeline(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline) {
        return bindPipeline(commandBuffer, pipelineBindPoint, vtPipeline);
    };

    inline VtResult vtCmdDispatchRayTracing(VtCommandBuffer commandBuffer, uint32_t x, uint32_t y) {
        return dispatchRayTracing(commandBuffer, x, y);
    };

    inline VtResult vtCmdCopyBuffer(VtCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        auto vctr = makeVector<vk::BufferCopy>((vk::BufferCopy *)pRegions, regionCount);
        return cmdCopyBufferL(commandBuffer, srcBuffer, dstBuffer, vctr, commandBarrier);
    };

    inline VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAcceleratorSet accelerator) {
        return bindAccelerator(commandBuffer, accelerator);
    };

    inline VtResult vtCmdBindVertexAssembly(VtCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly) {
        return bindVertexAssembly(commandBuffer, vertexAssembly);
    };


    inline VtResult vtCmdBuildVertexAssembly(VtCommandBuffer commandBuffer /*,  */) {
        return buildVertexSet(commandBuffer);
    };

    inline VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */) {
        return buildAccelerator(commandBuffer);
    };


    inline VtResult vtCmdBindVertexInputSets(VtCommandBuffer commandBuffer, uint32_t setCount, const VtVertexInputSet * sets) {
        return bindVertexInputs(commandBuffer, makeVector<std::shared_ptr<VertexInputSet>>((std::shared_ptr<VertexInputSet>*)sets, setCount));
    };

    // planned: roling by VtEntryUsageFlags
    inline VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials) {
        return bindMaterialSet(commandBuffer, usageIn, materials);
    };

    inline VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
        return bindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, makeVector<VkDescriptorSet>(pDescriptorSets, descriptorSetCount));
    };



    inline VtResult vtConvertPhysicalDevice(VtInstance vtInstance, VkPhysicalDevice vkPhysicalDevice, VtPhysicalDevice * vtPhysicalDevice) {
        return convertPhysicalDevice(vtInstance, vkPhysicalDevice, *vtPhysicalDevice);
    };

    inline VtResult vtConvertInstance(VkInstance vkInstance, const VtInstanceConversionInfo * cinfo, VtInstance * vtInstance) {
        return convertInstance(vkInstance, *cinfo, *vtInstance);
    };  


    // radix sort API
    inline VtResult vtRadixSort(VtCommandBuffer commandBuffer, VkDescriptorSet radixInput, uint32_t primCount) {
        return radixSort(commandBuffer, radixInput, primCount);
    };


    // create device image and buffers
    inline VtResult vtCreateDeviceImage(VtDevice device, const VtDeviceImageCreateInfo * info, VtDeviceImage * image) { return createDeviceImage(device, *info, *image); };
    inline VtResult vtCreateDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceBuffer * buffer) { return createDeviceBuffer(device, *info, *buffer); };
    inline VtResult vtCreateHostToDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtHostToDeviceBuffer * buffer) { return createHostToDeviceBuffer(device, *info, *buffer); };
    inline VtResult vtCreateDeviceToHostBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceToHostBuffer * buffer) { return createDeviceToHostBuffer(device, *info, *buffer); };

};
