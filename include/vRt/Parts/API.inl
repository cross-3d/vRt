#pragma once
#include "Vulkan.inl"

namespace vt { // store in official namespace
    // Description structs for make vRt objects
    // Note: structures already have default headers for identifying

    struct VtInstanceCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        const void* pNext = nullptr;
        VkInstance vkInstance;
    };

    struct VtDeviceCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        const void* pNext = nullptr;
        VtDevice vtDevice;
    };

    struct VtRayTracingCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO;
        const void* pNext = nullptr;
        VtDevice vtDevice;
    };

    struct VtDeviceConvertInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO;
        const void* pNext = nullptr;
        VtPhysicalDevice vtPhysicalDevice;
    };

    struct VtPhysicalDeviceConvertInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO;
        const void* pNext = nullptr;
        VtInstance vtInstance;
    };

    struct VtRayTracingPipelineCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO;
        const void* pNext = nullptr;
        VtDevice vtDevice;
    };


    // any other vertex accessors can be used by attributes
    struct VtVertexAccessor {
        //VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ACCESSOR;
        //const void* pNext = nullptr;
        uint32_t bufferViewID = 0;
        uint32_t byteOffset = 0;
        union {
            uint32_t components : 2, type : 4, normalized : 1;
            VtFormat format;
        };
    };


    // any other vertex bindings can be used by attributes
    struct VtVertexRegionBinding {
        //VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_REGION_BINDING;
        //const void* pNext = nullptr;
        //uint32_t byteStride = 0;
        uint32_t byteOffset = 0;
        uint32_t byteSize = 0;
        //VkBufferCopy bufferRegion; // import from Vulkan
    };


    // buffer view
    struct VtVertexBufferView {
        uint32_t regionID = 0;
        uint32_t byteOffset = 0;
        uint32_t byteStride = 0;
    };


    // attribute binding
    struct VtVertexAttributeBinding {
        //VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ATTRIBUTE_BINDING;
        //const void* pNext = nullptr;
        uint32_t attributeBinding = 0;
        uint32_t accessorID = 0;
    };


    struct VtVertexInputCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO;
        const void* pNext = nullptr;

        // all sources buffer
        VkBuffer sourceBuffer;

        // use buffers for shorter access
        //VkBuffer regionsBuffer;
        //VkBuffer accessorsBuffer;

        // bindings regions
        VtVertexRegionBinding * pBufferRegionBindings = nullptr;
        uint32_t bufferRegionCount = 0;

        // accessor regions
        VtVertexAccessor * pBufferAccessors = nullptr;
        uint32_t bufferAccessorCount = 0;

        // attribute bindings (will stored in special patterned image buffer)
        VtVertexAttributeBinding * pBufferAttributeBindings = nullptr;
        uint32_t attributeBindingCount = 0;

        VtVertexBufferView * pBufferViews = nullptr;
        uint32_t bufferViewCount = 0;

        // where from must got vertex and indices
        uint32_t verticeAccessor = 0;
        uint32_t indiceAccessor = 0xFFFFFFFF; // has no indice accessor

        // supported vertex topology
        VtTopologyType topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;
        uint32_t materialID = 0; // material ID for identify in hit shader
    };



    struct VtMaterialsInputCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_MATERIALS_INPUT_CREATE_INFO;
        const void* pNext = nullptr;

        // immutable images (textures)
        const VkDescriptorImageInfo* pImages;
        uint32_t imageCount;

        // immutable samplers
        const VkSampler* pSamplers;
        uint32_t samplerCount;

        // virtual combined textures with samplers
        const uint64_t* pImageSamplerCombinations; // uint32 for images and next uint32 for sampler ID's
        uint32_t imageSamplerCount;

        // user defined materials 
        VkBuffer materialDescriptionsBuffer; // buffer for user material descriptions
        const uint32_t* pMaterialDescriptionIDs;
        uint32_t materialCount;
    };



    struct VtAcceleratorCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ACCELERATOR_CREATE_INFO;
        const void* pNext = nullptr;
    };



    // system vectors of ray tracers
    struct VtVec4 { float x, y, z, w; };
    struct VtVec3 { float x, y, z; };
    struct VtVec2 { float x, y; };
    struct VtUVec2 { uint32_t x, y; };


    // in future planned custom ray structures support
    // in current moment we will using 32-byte standard structuring
    struct VtRay {
        VtVec3 origin; // position state (in 3D)
        int32_t hitID; // id of intersection hit (-1 is missing)
        VtVec2 cdirect; // polar direction
        uint32_t _indice; // reserved for indice in another ray system
        uint16_t hf_r, hf_g, hf_b, bitfield;
    };


    struct VtNamedCombinedImagePair {
        uint32_t textureID, samplerID;
    };

    struct VtVirtualCombinedImage {
        union {
            VtNamedCombinedImagePair pair;
            uint64_t combined;
        };

        operator VtNamedCombinedImagePair&() { return pair; }; // return editable
        operator uint64_t() const { return combined; }; // return paired
    };



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
