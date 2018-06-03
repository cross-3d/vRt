#pragma once
#include "Vulkan.inl"
#include "Enums.inl"
#include "Structures.inl"

namespace vt { // store in official namespace

    // wrapped API for create device, instance and physical device
    VtResult vtCreateInstance(const VtInstanceCreateInfo * vtInstanceCreateInfo, VtInstance * vtInstance);
    VtResult vtEnumeratePhysicalDevices(VtInstance vtInstance, uint32_t* pPhysicalDeviceCount, VtPhysicalDevice* pPhysicalDevices);
    VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VtDeviceCreateInfo * vtDeviceCreateInfo, VtDevice * vtDevice);

    // conversion API objects from VK
    VtResult vtConvertPhysicalDevice(VkPhysicalDevice vkPhysicalDevice, const VtPhysicalDeviceConvertInfo * vtPhysicalDeviceConvertInfo, VtPhysicalDevice * vtPhysicalDevice);
    VtResult vtConvertDevice(VkDevice vkDevice, const VtDeviceConvertInfo * vtDeviceConvertInfo, VtDevice * vtDevice);

    // create ray tracing instance
    //VtResult vtCreateRayTracing(const VtRayTracingCreateInfo * vtRayTracingCreateInfo, VtRayTracing * vtRayTracing); // unknown, will saved or not

    // create ray tracing pipeline
    VtResult vtCreateRayTracingPipeline(const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineCreateInfo, VtPipeline * vtPipeline);

    // create ray tracing accelerator structure
    VtResult vtCreateAccelerator(const VtAcceleratorCreateInfo * vtAcceleratorCreateInfo, VtAccelerator * vtAccelerator);

    // make vertex input instance
    VtResult vtCreateVertexInputSource(const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInput * vtVertexInput);

    // make descriptor input 
    VtResult vtCreateMaterialsInput(const VtMaterialsInputCreateInfo * vtMaterialsCreateInfo, VtMaterialsInput * materialsInput);


    // make command buffer capable with ray tracing factory (VtCommandBuffer)
    VtResult vtQueryCommandInterface(VkCommandBuffer commandBuffer, VtDevice device, VtCommandBuffer * vtCommandBuffer);

    // bind ray tracing pipeline
    VtResult vtCmdBindPipeline(VtCommandBuffer cmdBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline);

    // dispatch ray tracing
    VtResult vtCmdDispatchRayTracing(VtCommandBuffer cmdBuffer, uint32_t x = 1, uint32_t y = 1);

    // use compute capable copy buffer
    VtResult vtCmdCopyBuffer(VtCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);



    // bind vertex input for accelerator builders (accelerators itself also will store these data)
    VtResult vtCmdBindVertexInput(VtCommandBuffer commandBuffer, uint32_t vertexInputCount, const VtVertexInput * vertexInput);
    //VtResult vtCmdBindVertexInput(VtCommandBuffer commandBuffer, VtVertexInput vertexInput);

    // bind accelerator structure for building/ray tracing
    VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAccelerator accelerator);

    // pre-build vertex input in accelerator structure
    VtResult vtCmdBuildVertexInput(VtCommandBuffer commandBuffer /*,  */);

    // pre-build material sets
    VtResult vtCmdMaterialsInput(VtCommandBuffer commandBuffer /*,  */);

    // build accelerator structure command
    VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */);

    // descriptorSet = "0" will blocked by ray tracing system
    VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);

    // bind materials input (for closest hit shaders)
    VtResult vtCmdBindMaterialsInput(VtCommandBuffer commandBuffer, VtMaterialsInput materials);

};
