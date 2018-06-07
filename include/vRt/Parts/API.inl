#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"
#include "HandlersDef.inl"
#include "StructuresDef.inl"
#include "Enums.inl"

namespace vt { // store in official namespace

    // wrapped API for create device, instance and physical device 
    // planned wrapped instance creator with required extensions support
    inline VtResult vtEnumeratePhysicalDevices(VtInstance vtInstance, uint32_t* pPhysicalDeviceCount, VtPhysicalDevice* pPhysicalDevices);
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice);

    // conversion API objects from VK
    inline VtResult vtConvertInstance(VkDevice vkInstance, const VtInstanceConversionInfo * vtInstanceCreateInfo, VtInstance * vtInstance);
    inline VtResult vtConvertPhysicalDevice(VkPhysicalDevice vkPhysicalDevice, const VtPhysicalDeviceConversionInfo * vtPhysicalDeviceConvertInfo, VtPhysicalDevice * vtPhysicalDevice);
    inline VtResult vtConvertDevice(VkDevice vkDevice, const VtDeviceConversionInfo * vtDeviceConvertInfo, VtDevice * vtDevice);

    // create ray tracing pipeline
    inline VtResult vtCreateRayTracingPipeline(VtDevice device, const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineCreateInfo, VtPipeline * vtPipeline);

    // create ray tracing accelerator structure
    inline VtResult vtCreateAccelerator(VtDevice device, const VtAcceleratorCreateInfo * vtAcceleratorCreateInfo, VtAccelerator * accelerator);

    // make descriptor input 
    inline VtResult vtCreateMaterialSet(VtDevice vtDevice, const VtMaterialSetCreateInfo * vtMaterialsCreateInfo, VtMaterialSet * materialsInput);

    // make vertex input set
    inline VtResult vtCreateVertexInputSet(VtDevice vtDevice, const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInputSet * vertexInputSet);


    // make command buffer capable with ray tracing factory (VtCommandBuffer)
    inline VtResult vtQueryCommandInterface(VkCommandBuffer commandBuffer, VtDevice device, VtCommandBuffer * vtCommandBuffer);

    // bind ray tracing pipeline
    inline VtResult vtCmdBindPipeline(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline);

    // dispatch ray tracing
    inline VtResult vtCmdDispatchRayTracing(VtCommandBuffer commandBuffer, uint32_t x = 1, uint32_t y = 1);

    // use compute capable copy buffer
    inline VtResult vtCmdCopyBuffer(VtCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);


    // bind accelerator structure for building/ray tracing
    inline VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAccelerator accelerator);

    // pre-build vertex input in accelerator structure
    inline VtResult vtCmdBuildVertexInputs(VtCommandBuffer commandBuffer /*,  */);

    // build accelerator structure command
    inline VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */);

    // descriptorSet = "0" and "1" will blocked by ray tracing system
    inline VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);

    // bind materials set 
    inline VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials);

    // image barrier (with state Vulkan API command buffer)
    inline VtResult vtCmdImageBarrier(VkCommandBuffer commandBuffer, VtDeviceImage image);
};
