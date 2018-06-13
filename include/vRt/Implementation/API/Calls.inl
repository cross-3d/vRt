#pragma once

#include "../../vRt_subimpl.inl"
#include "./BuildAccelerator.inl"
#include "./RayTracing.inl"
#include "./RadixSort.inl"

namespace vt {
    using namespace _vt;

    // experimental implementation of "vtCreateDevice"
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice) {
        return createDevice(vtPhysicalDevice._vtPhysicalDevice, *deviceCreateInfo, vtDevice->_vtDevice);
    };

    inline VtResult vtCmdImageBarrier(VkCommandBuffer cmd, VtDeviceImage image) {
        return imageBarrier(cmd, image._vtDeviceImage);
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
        return createCommandBuffer(device, commandBuffer, *vtCommandBuffer);
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



    inline VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_RAY_TRACING) {
            commandBuffer._vtCommandBuffer->_boundDescriptorSets = makeVector<VkDescriptorSet>(pDescriptorSets, descriptorSetCount);
        }
        return VK_SUCCESS;
    };

    // planned: roling by VtEntryUsageFlags
    inline VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials) {
        commandBuffer._vtCommandBuffer->_materialSet = materials._vtMaterialSet;
        return VK_SUCCESS;
    };


};
