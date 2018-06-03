#pragma once
#include "Headers.inl"
#include "Enums.inl"
#include "Structures.inl"
#include "Handlers.inl"

namespace vt { // store in official namespace

    // wrapped API for create device, instance and physical device
    inline VtResult vtCreateInstance(const VtInstanceCreateInfo * vtInstanceCreateInfo, VtInstance * vtInstance);
    inline VtResult vtEnumeratePhysicalDevices(VtInstance vtInstance, uint32_t* pPhysicalDeviceCount, VtPhysicalDevice* pPhysicalDevices);
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VtDeviceCreateInfo * vtDeviceCreateInfo, VtDevice * vtDevice);

    // conversion API objects from VK
    inline VtResult vtConvertPhysicalDevice(VkPhysicalDevice vkPhysicalDevice, const VtPhysicalDeviceConvertInfo * vtPhysicalDeviceConvertInfo, VtPhysicalDevice * vtPhysicalDevice);
    inline VtResult vtConvertDevice(VkDevice vkDevice, const VtDeviceConvertInfo * vtDeviceConvertInfo, VtDevice * vtDevice);

    // create ray tracing instance
    //VtResult vtCreateRayTracing(const VtRayTracingCreateInfo * vtRayTracingCreateInfo, VtRayTracing * vtRayTracing); // unknown, will saved or not

    // create ray tracing pipeline
    inline VtResult vtCreateRayTracingPipeline(const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineCreateInfo, VtPipeline * vtPipeline);

    // create ray tracing accelerator structure
    inline VtResult vtCreateAccelerator(const VtAcceleratorCreateInfo * vtAcceleratorCreateInfo, VtAccelerator * vtAccelerator);

    // make vertex input instance
    inline VtResult vtCreateVertexInputSource(const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInput * vtVertexInput);

    // make descriptor input 
    inline VtResult vtCreateMaterialsInput(const VtMaterialsInputCreateInfo * vtMaterialsCreateInfo, VtMaterialsInput * materialsInput);


    // make command buffer capable with ray tracing factory (VtCommandBuffer)
    inline VtResult vtQueryCommandInterface(VkCommandBuffer commandBuffer, VtDevice device, VtCommandBuffer * vtCommandBuffer);

    // bind ray tracing pipeline
    inline VtResult vtCmdBindPipeline(VtCommandBuffer cmdBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline);

    // dispatch ray tracing
    inline VtResult vtCmdDispatchRayTracing(VtCommandBuffer cmdBuffer, uint32_t x = 1, uint32_t y = 1);

    // use compute capable copy buffer
    inline VtResult vtCmdCopyBuffer(VtCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);



    // bind vertex input for accelerator builders (accelerators itself also will store these data)
    inline VtResult vtCmdBindVertexInput(VtCommandBuffer commandBuffer, uint32_t vertexInputCount, const VtVertexInput * vertexInput);
    //inline VtResult vtCmdBindVertexInput(VtCommandBuffer commandBuffer, VtVertexInput vertexInput);

    // bind accelerator structure for building/ray tracing
    inline VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAccelerator accelerator);

    // pre-build vertex input in accelerator structure
    inline VtResult vtCmdBuildVertexInput(VtCommandBuffer commandBuffer /*,  */);

    // pre-build material sets
    inline VtResult vtCmdMaterialsInput(VtCommandBuffer commandBuffer /*,  */);

    // build accelerator structure command
    inline VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */);

    // descriptorSet = "0" will blocked by ray tracing system
    inline VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);

    // bind materials input (for closest hit shaders)
    inline VtResult vtCmdBindMaterialsInput(VtCommandBuffer commandBuffer, VtMaterialsInput materials);

};
