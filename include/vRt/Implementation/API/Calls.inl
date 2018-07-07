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
        return createPipelineLayout(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipelineLayout, VT_PIPELINE_LAYOUT_TYPE_RAYTRACING);
    };

    inline VtResult vtCreateVertexAssemblyPipelineLayout(VtDevice device, const VkPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout) {
        return createPipelineLayout(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipelineLayout, VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT);
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

    inline VtResult vtCreateVertexAssemblyPipeline(VtDevice device, const VtVertexAssemblyPipelineCreateInfo * vtVertexAssemblyPipelineCreateInfo, VtVertexAssemblyPipeline * vertexAssemblyPipeline) {
        return createVertexAssemblyPipeline(device, *vtVertexAssemblyPipelineCreateInfo, *vertexAssemblyPipeline);
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

    inline VtResult vtCmdDispatchRayTracing(VtCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t B) {
        return dispatchRayTracing(commandBuffer, x, y, B);
    };

    inline VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAcceleratorSet accelerator) {
        return bindAccelerator(commandBuffer, accelerator);
    };

    inline VtResult vtCmdBindVertexAssembly(VtCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly) {
        return bindVertexAssembly(commandBuffer, vertexAssembly);
    };

    inline VtResult vtCmdVertexAssemblyBarrier(VkCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly) {
        return cmdVertexAssemblyBarrier(commandBuffer, vertexAssembly);
    };


    inline VtResult vtCmdUpdateVertexAssembly(VtCommandBuffer commandBuffer, uint32_t inputSet, bool multiple) {
        return updateVertexSet(commandBuffer, inputSet, multiple);
    };

    inline VtResult vtCmdBuildVertexAssembly(VtCommandBuffer commandBuffer, std::function<void(VkCommandBuffer, int, VtUniformBlock&)> cb) {
        return buildVertexSet(commandBuffer, cb);
    };

    inline VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */) {
        return buildAccelerator(commandBuffer);
    };


    inline VtResult vtCmdBindVertexInputSets(VtCommandBuffer commandBuffer, uint32_t setCount, const VtVertexInputSet * sets) {
        std::vector<std::shared_ptr<VertexInputSet>> inputSets;
        for (int i = 0; i < setCount;i++) {
            inputSets.push_back(sets[i]);
        }
        return bindVertexInputs(commandBuffer, inputSets);
    };

    // planned: roling by VtEntryUsageFlags
    inline VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials) {
        return bindMaterialSet(commandBuffer, usageIn, materials);
    };

    inline VtResult vtCmdBindDescriptorSetsPerVertexInput(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t vertexInputIdx, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
        return bindDescriptorSetsPerVertexInput(commandBuffer, pipelineBindPoint, layout, vertexInputIdx, firstSet, makeVector<VkDescriptorSet>(pDescriptorSets, descriptorSetCount));
    };

    inline VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
        return bindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, makeVector<VkDescriptorSet>(pDescriptorSets, descriptorSetCount));
    };

    // planned to merge into dedicated implementation
    inline VtResult vtCmdBindRayTracingSet(VtCommandBuffer commandBuffer, VtRayTracingSet rtset) {
        commandBuffer->_rayTracingSet = std::shared_ptr<_vt::RayTracingSet>(rtset);
        return VK_SUCCESS;
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




    // between buffers
    inline VtResult vtCmdCopyDeviceBuffer(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, makeVector<vk::BufferCopy>((const vk::BufferCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between buffers and host
    inline VtResult vtCmdCopyHostToDeviceBuffer(VkCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        cmdCopyBufferFromHost(commandBuffer, srcBuffer, dstBuffer, makeVector<vk::BufferCopy>((const vk::BufferCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    inline VtResult vtCmdCopyDeviceBufferToHost(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        cmdCopyBufferToHost(commandBuffer, srcBuffer, dstBuffer, makeVector<vk::BufferCopy>((const vk::BufferCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between host and images
    inline VtResult vtCmdCopyHostToDeviceImage(VkCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        cmdCopyBufferToImage<VMA_MEMORY_USAGE_CPU_TO_GPU>(commandBuffer, srcBuffer, dstImage, makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };
    inline VtResult vtCmdCopyDeviceImageToHost(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>(commandBuffer, srcImage, dstBuffer, makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between buffers and images 
    inline VtResult vtCmdCopyDeviceBufferToImage(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        cmdCopyBufferToImage<VMA_MEMORY_USAGE_GPU_ONLY>(commandBuffer, srcBuffer, dstImage, makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };
    inline VtResult vtCmdCopyDeviceImageToBuffer(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_ONLY>(commandBuffer, srcImage, dstBuffer, makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between images 
    inline VtResult vtCmdCopyDeviceImage(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceImage dstImage, uint32_t regionCount, const VkImageCopy* pRegions) {
        cmdCopyDeviceImage(commandBuffer, srcImage, dstImage, makeVector<vk::ImageCopy>((const vk::ImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };


    /*
    template <class T>
    inline VtResult vtSetBufferSubData(const std::vector<T> &hostdata, VtHostToDeviceBuffer buffer, intptr_t offset) {
        setBufferSubData<T, VMA_MEMORY_USAGE_CPU_TO_GPU>(hostdata, buffer, offset); return VK_SUCCESS;
    };

    template <class T>
    inline VtResult vtGetBufferSubData(VtDeviceToHostBuffer buffer, std::vector<T> &hostdata, intptr_t offset) {
        getBufferSubData<T, VMA_MEMORY_USAGE_GPU_TO_CPU>(buffer, hostdata, offset); return VK_SUCCESS;
    };

    template <class T>
    inline std::vector<T> vtGetBufferSubData(VtDeviceToHostBuffer buffer, size_t count, intptr_t offset) {
        return getBufferSubData<T, VMA_MEMORY_USAGE_GPU_TO_CPU>(buffer, count, offset);
    };
    */

};
