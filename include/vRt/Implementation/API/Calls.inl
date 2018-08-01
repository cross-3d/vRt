#pragma once

#include "../../vRt_subimpl.inl"
#include "./BuildAccelerator.inl"
#include "./RayTracing.inl"
#include "./RadixSort.inl"

namespace vrt {
    //using namespace _vt;

    VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice) {
        return _vt::createDevice(vtPhysicalDevice, *deviceCreateInfo, vtDevice->_vtDevice);
    };

    VtResult vtConvertDevice(VtPhysicalDevice vtPhysicalDevice, VkDevice vkDevice, const VtArtificalDeviceExtension * vtDeviceExtension, VtDevice * vtDevice) {
        return _vt::convertDevice(vkDevice, vtPhysicalDevice, *vtDeviceExtension, *vtDevice);
    };

    VtResult vtCmdImageBarrier(VkCommandBuffer cmd, VtDeviceImage image) {
        return _vt::imageBarrier(cmd, image);
    };

    VtResult vtCreateRayTracingPipelineLayout(VtDevice device, const VtPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout) {
        return _vt::createPipelineLayout(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipelineLayout, VT_PIPELINE_LAYOUT_TYPE_RAYTRACING);
    };

    VtResult vtCreateVertexAssemblyPipelineLayout(VtDevice device, const VtPipelineLayoutCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipelineLayout * vtPipelineLayout) {
        return _vt::createPipelineLayout(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipelineLayout, VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT);
    };

    VtResult vtCreateRayTracingPipeline(VtDevice device, const VtRayTracingPipelineCreateInfo * vtRayTracingPipelineLayoutCreateInfo, VtPipeline * vtPipeline) {
        return _vt::createRayTracingPipeline(device, *vtRayTracingPipelineLayoutCreateInfo, *vtPipeline);
    };

    VtResult vtCreateRayTracingSet(VtDevice device, const VtRayTracingSetCreateInfo * vtSetCreateInfo, VtRayTracingSet * vtSet) {
        return _vt::createRayTracingSet(device, *vtSetCreateInfo, *vtSet);
    };

    VtResult vtCreateAccelerator(VtDevice device, const VtAcceleratorSetCreateInfo * vtAcceleratorCreateInfo, VtAcceleratorSet * accelerator) {
        return _vt::createAcceleratorSet(device, *vtAcceleratorCreateInfo, *accelerator);
    };

    VtResult vtCreateVertexAssembly(VtDevice device, const VtVertexAssemblySetCreateInfo * vtVertexAssemblyCreateInfo, VtVertexAssemblySet * vertexAssembly) {
        return _vt::createVertexAssemblySet(device, *vtVertexAssemblyCreateInfo, *vertexAssembly);
    };

    VtResult vtCreateVertexAssemblyPipeline(VtDevice device, const VtVertexAssemblyPipelineCreateInfo * vtVertexAssemblyPipelineCreateInfo, VtVertexAssemblyPipeline * vertexAssemblyPipeline) {
        return _vt::createVertexAssemblyPipeline(device, *vtVertexAssemblyPipelineCreateInfo, *vertexAssemblyPipeline);
    };

    VtResult vtCreateMaterialSet(VtDevice device, const VtMaterialSetCreateInfo * vtMaterialsCreateInfo, VtMaterialSet * materialsInput) {
        return _vt::createMaterialSet(device, *vtMaterialsCreateInfo, *materialsInput);
    };

    VtResult vtCreateVertexInputSet(VtDevice device, const VtVertexInputCreateInfo * vtVertexInputCreateInfo, VtVertexInputSet * vertexInputSet) {
        return _vt::createVertexInputSet(device, *vtVertexInputCreateInfo, *vertexInputSet);
    };

    VtResult vtQueryCommandInterface(VtDevice device, VkCommandBuffer commandBuffer, VtCommandBuffer * vtCommandBuffer) {
        return _vt::queryCommandBuffer(device, commandBuffer, *vtCommandBuffer);
    };

    VtResult vtCmdBindPipeline(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipeline vtPipeline) {
        return _vt::bindPipeline(commandBuffer, pipelineBindPoint, vtPipeline);
    };

    VtResult vtCmdDispatchRayTracing(VtCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t B) {
        return _vt::dispatchRayTracing(commandBuffer, x, y, B);
    };

    VtResult vtCmdBindAccelerator(VtCommandBuffer commandBuffer, VtAcceleratorSet accelerator) {
        return _vt::bindAccelerator(commandBuffer, accelerator);
    };

    VtResult vtCmdBindVertexAssembly(VtCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly) {
        return _vt::bindVertexAssembly(commandBuffer, vertexAssembly);
    };

    VtResult vtCmdVertexAssemblyBarrier(VkCommandBuffer commandBuffer, VtVertexAssemblySet vertexAssembly) {
        return _vt::cmdVertexAssemblyBarrier(commandBuffer, vertexAssembly);
    };

    VtResult vtCmdUpdateVertexAssembly(VtCommandBuffer commandBuffer, uint32_t inputSet, bool multiple, bool useInstance, const std::function<void(VkCommandBuffer, int, VtUniformBlock&)>& cb) {
        return _vt::updateVertexSet(commandBuffer, inputSet, multiple, useInstance, cb);
    };

    VtResult vtCmdBuildVertexAssembly(VtCommandBuffer commandBuffer, bool useInstance, const std::function<void(VkCommandBuffer, int, VtUniformBlock&)>& cb) {
        return _vt::buildVertexSet(commandBuffer, useInstance, cb);
    };

    VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */) {
        return _vt::buildAccelerator(commandBuffer);
    };

    VtResult vtCmdBindVertexInputSets(VtCommandBuffer commandBuffer, uint32_t setCount, const VtVertexInputSet * sets) {
        std::vector<std::shared_ptr<_vt::VertexInputSet>> inputSets;
        for (uint32_t i = 0; i < setCount;i++) {
            inputSets.push_back(sets[i]._vtVertexInputSet);
        }
        return _vt::bindVertexInputs(commandBuffer, inputSets);
    };

    // planned: roling by VtEntryUsageFlags
    VtResult vtCmdBindMaterialSet(VtCommandBuffer commandBuffer, VtEntryUsageFlags usageIn, VtMaterialSet materials) {
        return _vt::bindMaterialSet(commandBuffer, usageIn, materials);
    };

    VtResult vtCmdBindDescriptorSetsPerVertexInput(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t vertexInputIdx, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
        return _vt::bindDescriptorSetsPerVertexInput(commandBuffer, pipelineBindPoint, layout, vertexInputIdx, firstSet, _vt::makeVector<VkDescriptorSet>(pDescriptorSets, descriptorSetCount));
    };

    VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
        return _vt::bindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, _vt::makeVector<VkDescriptorSet>(pDescriptorSets, descriptorSetCount));
    };

    // planned to merge into dedicated implementation
    VtResult vtCmdBindRayTracingSet(VtCommandBuffer commandBuffer, VtRayTracingSet rtset) {
        commandBuffer->_rayTracingSet = rtset._vtRTSet;
        return VK_SUCCESS;
    };

    VtResult vtConvertPhysicalDevice(VtInstance vtInstance, VkPhysicalDevice vkPhysicalDevice, VtPhysicalDevice * vtPhysicalDevice) {
        return _vt::convertPhysicalDevice(vtInstance, vkPhysicalDevice, *vtPhysicalDevice);
    };

    VtResult vtConvertInstance(VkInstance vkInstance, const VtInstanceConversionInfo * cinfo, VtInstance * vtInstance) {
        return _vt::convertInstance(vkInstance, *cinfo, *vtInstance);
    };

    // radix sort API
    VtResult vtRadixSort(VtCommandBuffer commandBuffer, VkDescriptorSet radixInput, uint32_t primCount) {
        return _vt::radixSort(commandBuffer, radixInput, primCount);
    };

    // create device image and buffers
    VtResult vtCreateDeviceImage(VtDevice device, const VtDeviceImageCreateInfo * info, VtDeviceImage * image) { return _vt::createDeviceImage(device, *info, *image); };
    VtResult vtCreateDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceBuffer * buffer) { return _vt::createDeviceBuffer(device, *info, *buffer); };
    VtResult vtCreateHostToDeviceBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtHostToDeviceBuffer * buffer) { return _vt::createHostToDeviceBuffer(device, *info, *buffer); };
    VtResult vtCreateDeviceToHostBuffer(VtDevice device, const VtDeviceBufferCreateInfo * info, VtDeviceToHostBuffer * buffer) { return _vt::createDeviceToHostBuffer(device, *info, *buffer); };

    // between buffers
    VtResult vtCmdCopyDeviceBuffer(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        _vt::cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, _vt::makeVector<vk::BufferCopy>((const vk::BufferCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between buffers and host
    VtResult vtCmdCopyHostToDeviceBuffer(VkCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        _vt::cmdCopyBufferFromHost(commandBuffer, srcBuffer, dstBuffer, _vt::makeVector<vk::BufferCopy>((const vk::BufferCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    VtResult vtCmdCopyDeviceBufferToHost(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
        _vt::cmdCopyBufferToHost(commandBuffer, srcBuffer, dstBuffer, _vt::makeVector<vk::BufferCopy>((const vk::BufferCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between host and images
    VtResult vtCmdCopyHostToDeviceImage(VkCommandBuffer commandBuffer, VtHostToDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        _vt::cmdCopyBufferToImage<VMA_MEMORY_USAGE_CPU_TO_GPU>(commandBuffer, srcBuffer, dstImage, _vt::makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };
    VtResult vtCmdCopyDeviceImageToHost(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceToHostBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        _vt::cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>(commandBuffer, srcImage, dstBuffer, _vt::makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between buffers and images 
    VtResult vtCmdCopyDeviceBufferToImage(VkCommandBuffer commandBuffer, VtDeviceBuffer srcBuffer, VtDeviceImage dstImage, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        _vt::cmdCopyBufferToImage<VMA_MEMORY_USAGE_GPU_ONLY>(commandBuffer, srcBuffer, dstImage, _vt::makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };
    VtResult vtCmdCopyDeviceImageToBuffer(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
        _vt::cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_ONLY>(commandBuffer, srcImage, dstBuffer, _vt::makeVector<vk::BufferImageCopy>((const vk::BufferImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // between images 
    VtResult vtCmdCopyDeviceImage(VkCommandBuffer commandBuffer, VtDeviceImage srcImage, VtDeviceImage dstImage, uint32_t regionCount, const VkImageCopy* pRegions) {
        _vt::cmdCopyDeviceImage(commandBuffer, srcImage, dstImage, _vt::makeVector<vk::ImageCopy>((const vk::ImageCopy *)pRegions, regionCount)); return VK_SUCCESS;
    };

    // get all possible required extensions
    VtResult vtGetRequiredExtensions(VkDevice vkDevice, uint32_t* extCount, const char ** extensionNames) {
        if (extensionNames) {
            memcpy(extensionNames, _vt::raytracingRequiredExtensions.data(), _vt::raytracingRequiredExtensions.size());
        }
        if (extCount) {
            *extCount = _vt::raytracingRequiredExtensions.size();
        }
        return VK_SUCCESS;
    }


    /*
    template <class T>
    VtResult vtSetBufferSubData(const std::vector<T> &hostdata, VtHostToDeviceBuffer buffer, intptr_t offset) {
        setBufferSubData<T, VMA_MEMORY_USAGE_CPU_TO_GPU>(hostdata, buffer, offset); return VK_SUCCESS;
    };

    template <class T>
    VtResult vtGetBufferSubData(VtDeviceToHostBuffer buffer, std::vector<T> &hostdata, intptr_t offset) {
        getBufferSubData<T, VMA_MEMORY_USAGE_GPU_TO_CPU>(buffer, hostdata, offset); return VK_SUCCESS;
    };

    template <class T>
    inline std::vector<T> vtGetBufferSubData(VtDeviceToHostBuffer buffer, size_t count, intptr_t offset) {
        return getBufferSubData<T, VMA_MEMORY_USAGE_GPU_TO_CPU>(buffer, count, offset);
    };
    */

};
