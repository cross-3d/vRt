#pragma once

#include "Headers.inl"
#include "Enums.inl"
#include "StructuresDef.inl"
#include "HandlersDef.inl"

namespace vrt { // store in official namespace

    // wrapped API for create device, instance and physical device 
    // planned wrapped instance creator with required extensions support
    extern VtResult vtEnumeratePhysicalDevices(VtInstance vtInstance, uint32_t* pPhysicalDeviceCount, VtPhysicalDevice* pPhysicalDevices);
    extern VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice);
    extern VtResult vtGetRequiredExtensions(VkDevice vkDevice, uint32_t* extCount, const char ** extensionNames = nullptr);

    // conversion API objects from VK
    extern VtResult vtConvertInstance(VkInstance vkInstance, const VtInstanceConversionInfo * vtInstanceCreateInfo, VtInstance * vtInstance);
    extern VtResult vtConvertPhysicalDevice(VtInstance vtInstance, VkPhysicalDevice vkPhysicalDevice, VtPhysicalDevice * vtPhysicalDevice);
    extern VtResult vtConvertDevice(VtPhysicalDevice vtPhysicalDevice, VkDevice vkDevice, const VtArtificalDeviceExtension * vtDeviceExtension, VtDevice * vtDevice);

    // create ray tracing pipelineLayout 
    extern VtResult vtCreateRayTracingPipelineLayout(VtDevice device, const VtPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout);

    // create vertex assembly pipeline
    extern VtResult vtCreateAttributePipeline(VtDevice device, const VtAttributePipelineCreateInfo * vtAssemblyPipelineCreateInfo, VtAttributePipeline * vertexAssemblyPipeline);

    // create vertex assembly pipelineLayout
    extern VtResult vtCreateAssemblyPipelineLayout(VtDevice device, const VtPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout);

    // create ray tracing storage
    extern VtResult vtCreateRayTracingSet(VtDevice device, const VtRayTracingSetCreateInfo * vtSetCreateInfo, VtRayTracingSet * vtSet);

    // create ray tracing pipeline
    extern VtResult vtCreateRayTracingPipeline(VtDevice device, const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineCreateInfo, VtPipeline * vtPipeline);

    // create ray tracing accelerator structure
    extern VtResult vtCreateAccelerator(VtDevice device, const VtAcceleratorSetCreateInfo * vtAcceleratorCreateInfo, VtAcceleratorSet * accelerator);

    // create ray tracing accelerator structure
    extern VtResult vtCreateVertexAssembly(VtDevice device, const VtVertexAssemblySetCreateInfo * assemblyPipelineCreateInfo, VtVertexAssemblySet * vertexAssembly);

    // make descriptor input 
    extern VtResult vtCreateMaterialSet(VtDevice device, const VtMaterialSetCreateInfo * vtMaterialsCreateInfo, VtMaterialSet * materialsInput);

    // make vertex input set
    extern VtResult vtCreateVertexInputSet(VtDevice device, const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInputSet * vertexInputSet);

    // make command buffer capable with ray tracing factory (VtCommandBuffer)
    extern VtResult vtQueryCommandInterface(VtDevice device, VkCommandBuffer commandBuffer, VtCommandBuffer * vtCommandBuffer);

    // bind ray tracing pipeline
    extern VtResult vtCmdBindPipeline(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline);

    // dispatch ray tracing
    extern VtResult vtCmdDispatchRayTracing(VtCommandBuffer commandBuffer, uint32_t x = 1, uint32_t y = 1, uint32_t B = 1);

    // bind accelerator structure for building/ray tracing
    extern VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAcceleratorSet accelerator);

    // bind accelerator structure for building/ray tracing
    extern VtResult vtCmdBindVertexAssembly(VtCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly);

    // update vertex assembly without needs to full rebuilding vertex set 
    extern VtResult vtCmdUpdateVertexAssembly(VtCommandBuffer commandBuffer, uint32_t inputSet, bool multiple = false, bool useInstance = true, const std::function<void(VkCommandBuffer, int, VtUniformBlock&)>& cb = {});

    // pre-build vertex input in accelerator structure
    extern VtResult vtCmdBuildVertexAssembly(VtCommandBuffer commandBuffer, bool useInstance = true, const std::function<void(VkCommandBuffer, int, VtUniformBlock&)>& cb = {});

    // build accelerator structure command
    extern VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer, VkDeviceSize coveredCapacity = VK_WHOLE_SIZE);

    extern VtResult vtCmdBindRayTracingSet(VtCommandBuffer commandBuffer, VtRayTracingSet rtset);

    extern VtResult vtCmdBindDescriptorSetsPerVertexInput(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t vertexInputIdx, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);

    extern VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);

    // bind materials set 
    extern VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials);

    // bind vertex inputs 
    extern VtResult vtCmdBindVertexInputSets(VtCommandBuffer commandBuffer, uint32_t setCount, const VtVertexInputSet * sets);

    // radix sort API
    extern VtResult vtRadixSort(VtCommandBuffer commandBuffer, VkDescriptorSet radixInput, uint32_t primCount = 2);

    extern VtResult vtCreateDeviceImage(VtDevice device, const VtDeviceImageCreateInfo * info, VtDeviceImage * image);
    extern VtResult vtCreateDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceBuffer * buffer);
    extern VtResult vtCreateHostToDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtHostToDeviceBuffer * buffer);
    extern VtResult vtCreateDeviceToHostBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceToHostBuffer * buffer);

    // image barrier (with state Vulkan API command buffer)
    extern VtResult vtCmdImageBarrier(VkCommandBuffer commandBuffer, VtDeviceImage image);
    extern VtResult vtCmdVertexAssemblyBarrier(VkCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly);

    // between buffers
    extern VtResult vtCmdCopyDeviceBuffer(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);

    // between buffers and host
    extern VtResult vtCmdCopyHostToDeviceBuffer(VkCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
    extern VtResult vtCmdCopyDeviceBufferToHost(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);

    // between host and images
    extern VtResult vtCmdCopyHostToDeviceImage(VkCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    extern VtResult vtCmdCopyDeviceImageToHost(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    // between buffers and images 
    extern VtResult vtCmdCopyDeviceBufferToImage(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    extern VtResult vtCmdCopyDeviceImageToBuffer(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    // between images 
    extern VtResult vtCmdCopyDeviceImage(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceImage dstImage, uint32_t regionCount, const VkImageCopy* pRegions);


    template <class T>
    extern VtResult vtSetBufferSubData(const std::vector<T> &hostdata, VtHostToDeviceBuffer buffer, VkDeviceSize offset = 0);

    template <class T>
    extern VtResult vtGetBufferSubData(VtDeviceToHostBuffer buffer, std::vector<T> &hostdata, VkDeviceSize offset = 0);
    
    template <class T>
    extern inline std::vector<T> vtGetBufferSubData(VtDeviceToHostBuffer buffer, VkDeviceSize count = 1, VkDeviceSize offset = 0);
};
