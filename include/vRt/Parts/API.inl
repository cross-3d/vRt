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
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice);

    // conversion API objects from VK
    inline VtResult vtConvertInstance(VkInstance vkInstance, const VtInstanceConversionInfo * vtInstanceCreateInfo, VtInstance * vtInstance);
    inline VtResult vtConvertPhysicalDevice(VtInstance vtInstance, VkPhysicalDevice vkPhysicalDevice, VtPhysicalDevice * vtPhysicalDevice);
    inline VtResult vtConvertDevice(VtPhysicalDevice vtPhysicalDevice, VkDevice vkDevice, const VtArtificalDeviceExtension * vtDeviceExtension, VtDevice * vtDevice);

    // create ray tracing pipelineLayout 
    inline VtResult vtCreateRayTracingPipelineLayout(VtDevice device, const VkPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout);


    // create ray tracing storage
    inline VtResult vtCreateRayTracingSet(VtDevice device, const VtRayTracingSetCreateInfo * vtSetCreateInfo, VtRayTracingSet * vtSet);

    // create ray tracing pipeline
    inline VtResult vtCreateRayTracingPipeline(VtDevice device, const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineCreateInfo, VtPipeline * vtPipeline);

    // create ray tracing accelerator structure
    inline VtResult vtCreateAccelerator(VtDevice device, const VtAcceleratorSetCreateInfo * vtAcceleratorCreateInfo, VtAcceleratorSet * accelerator);

    // create ray tracing accelerator structure
    inline VtResult vtCreateVertexAssembly(VtDevice device, const VtVertexAssemblySetCreateInfo * vtVertexAssemblyCreateInfo, VtVertexAssemblySet * vertexAssembly);

    // make descriptor input 
    inline VtResult vtCreateMaterialSet(VtDevice device, const VtMaterialSetCreateInfo * vtMaterialsCreateInfo, VtMaterialSet * materialsInput);

    // make vertex input set
    inline VtResult vtCreateVertexInputSet(VtDevice device, const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInputSet * vertexInputSet);


    // make command buffer capable with ray tracing factory (VtCommandBuffer)
    inline VtResult vtQueryCommandInterface(VtDevice device, VkCommandBuffer commandBuffer, VtCommandBuffer * vtCommandBuffer);

    // bind ray tracing pipeline
    inline VtResult vtCmdBindPipeline(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline);

    // dispatch ray tracing
    inline VtResult vtCmdDispatchRayTracing(VtCommandBuffer commandBuffer, uint32_t x = 1, uint32_t y = 1);

    // use compute capable copy buffer
    inline VtResult vtCmdCopyBuffer(VtCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);


    // bind accelerator structure for building/ray tracing
    inline VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAcceleratorSet accelerator);

    // bind accelerator structure for building/ray tracing
    inline VtResult vtCmdBindVertexAssembly(VtCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly);


    // pre-build vertex input in accelerator structure
    inline VtResult vtCmdBuildVertexAssembly(VtCommandBuffer commandBuffer /*,  */);

    // build accelerator structure command
    inline VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */);



    // descriptorSet = "0" and "1" will blocked by ray tracing system
    inline VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);

    // bind materials set 
    inline VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials);

    // bind vertex inputs 
    inline VtResult vtCmdBindVertexInputSets(VtCommandBuffer commandBuffer, uint32_t setCount, const VtVertexInputSet * sets);



    // image barrier (with state Vulkan API command buffer)
    inline VtResult vtCmdImageBarrier(VkCommandBuffer commandBuffer, VtDeviceImage image);


    // radix sort API
    inline VtResult vtRadixSort(VtCommandBuffer commandBuffer, VkDescriptorSet radixInput, uint32_t primCount = 2);


    inline VtResult vtCreateDeviceImage(VtDevice device, const VtDeviceImageCreateInfo * info, VtDeviceImage * image);
    inline VtResult vtCreateDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceBuffer * buffer);
    inline VtResult vtCreateHostToDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtHostToDeviceBuffer * buffer);
    inline VtResult vtCreateDeviceToHostBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceToHostBuffer * buffer);

    // between buffers
    inline VtResult vtCmdCopyDeviceBuffer(VtCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);

    // between buffers and host
    inline VtResult vtCmdCopyHostToDeviceBuffer(VtCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
    inline VtResult vtCmdCopyDeviceBufferToHost(VtCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);

    // between host and images
    inline VtResult vtCmdCopyHostToDeviceImage(VtCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    inline VtResult vtCmdCopyDeviceImageToHost(VtCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    // between buffers and images 
    inline VtResult vtCmdCopyDeviceBufferToImage(VtCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    inline VtResult vtCmdCopyDeviceImageToBuffer(VtCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    // between images 
    inline VtResult vtCmdCopyDeviceImage(VtCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceImage dstImage, uint32_t regionCount, const VkImageCopy* pRegions);
};
