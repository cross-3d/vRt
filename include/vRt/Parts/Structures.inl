#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"
#include "StructuresDef.inl"
#include "Enums.inl"
#include "Handlers.inl"

namespace vrt { // store in official namespace
    // Description structs for make vRt objects
    // Note: structures already have default headers for identifying

    // in general that is conversion
    struct VtInstanceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_INSTANCE_CONVERSION_INFO;
        const void* pNext = nullptr;
    };

    struct VtArtificalDeviceExtension {
        VtStructureType sType = VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION;
        const void* pNext = nullptr;
        uint32_t mainQueueFamily = 0;
        VkDeviceSize sharedCacheSize = 1024u * 1024u;
        VkDeviceSize maxPrimitives = 1024u * 256u;
#ifdef VRT_ENABLE_STRING_VIEW
        std::string_view shaderPath = "./";
#else
        std::string shaderPath = "./";
#endif
        VmaAllocator allocator = nullptr;
    };

    struct VtDeviceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CONVERSION_INFO;
        const void* pNext = nullptr;
        VtPhysicalDevice physicalDevice;
    };

    struct VtPhysicalDeviceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERSION_INFO;
        const void* pNext = nullptr;
        VtInstance instance;
    };

    struct VtRayTracingSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_SET_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize maxRays = 1920ull * 1200ull;
    };

    struct VtRayTracingPipelineCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO;
        const void* pNext = nullptr;

        // ray tracing based pipeline layout
        VtPipelineLayout pipelineLayout;

        const VkPipelineShaderStageCreateInfo * pGenerationModule = nullptr;
        const VkPipelineShaderStageCreateInfo * pClosestModules = nullptr;
        const VkPipelineShaderStageCreateInfo * pMissModules = nullptr;
        const VkPipelineShaderStageCreateInfo * pGroupModules = nullptr;

        uint32_t closestModuleCount = 0;
        uint32_t missModuleCount = 0;
        uint32_t groupModuleCount = 0;
    };

    struct VtPipelineLayoutCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        const void* pNext = nullptr;
        const VkPipelineLayoutCreateInfo * pGeneralPipelineLayout = nullptr;
        VkBool32 enableMaterialSet = true;
    };

    // use immutables in accelerator inputs
    // planned support of indirect const buffers
    struct VtVertexInputCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO;
        const void* pNext = nullptr;

        VtVertexAssemblyPipeline vertexAssemblyPipeline;
        VtTopologyType topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;

        // original block
        uint32_t primitiveCount = 0;
        uint32_t verticeAccessor = 0;
        uint32_t indiceAccessor = 0xFFFFFFFF; // has no indice accessor
        uint32_t materialID = 0; // material ID for identify in hit shader

        // additional clause (16.06.2018)
        uint32_t primitiveOffset = 0;
        uint32_t attributeOffset = 0;
        uint32_t attributeCount = 8;
        uint32_t materialAccessor = 0xFFFFFFFF;

        // default hit group
        union {
            uint32_t bitfield = 0u;
            VtPrimitiveBitfield bitfieldDetail;
        };

        // vertex data sources
        const VkBufferView * pSourceBuffers = nullptr;
        uint32_t sourceBufferCount = 0;

        // bindings regions
        VkBuffer bBufferRegionBindings = nullptr; // direct descriptor set bind
        uintptr_t bufferRegionByteOffset = 0;

        // accessor regions
        VkBuffer bBufferAccessors = nullptr; // direct descriptor set bind
        uintptr_t bufferAccessorByteOffset = 0;

        // attribute bindings (will stored in special patterned image buffer)
        VkBuffer bBufferAttributeBindings = nullptr; // direct descriptor set bind
        uintptr_t attributeByteOffset = 0; // for buffer maners

        // buffer views
        VkBuffer bBufferViews = nullptr; // direct descriptor set bind
        uintptr_t bufferViewByteOffset = 0;
    };

    // use as low level typed descriptor set
    struct VtMaterialSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_MATERIAL_SET_CREATE_INFO;
        const void* pNext = nullptr;

        // immutable images (textures)
        const VkDescriptorImageInfo* pImages = nullptr;
        uint32_t imageCount = 0;

        // immutable samplers
        const VkSampler* pSamplers = nullptr;
        uint32_t samplerCount = 0;

        // virtual combined textures with samplers (better use in buffers)
        const uint64_t* pImageSamplerCombinations = nullptr; // uint32 for images and next uint32 for sampler ID's 
        VkBuffer bImageSamplerCombinations = nullptr;
        uint32_t imageSamplerCount = 0;

        // user defined materials 
        VkBuffer bMaterialDescriptionsBuffer = nullptr;
        //const uint32_t* pMaterialDescriptionIDs = nullptr; // I don't remember why it need
        uint32_t materialCount = 0;
    };

    // 
    struct VtVertexAssemblyPipelineCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ASSEMBLY_PIPELINE_CREATE_INFO;
        const void* pNext = nullptr;

        //size_t maxPrimitives = 1024u * 256u;
        VtPipelineLayout pipelineLayout;
        VkPipelineShaderStageCreateInfo vertexAssemblyModule;
    };

    // 
    struct VtVertexAssemblySetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ASSEMBLY_SET_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize maxPrimitives = 1024ull * 256ull;
    };

    // 
    struct VtAcceleratorSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ACCELERATOR_SET_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize maxPrimitives = 1024ull * 256ull;
        VkBuffer bvhMetaBuffer = nullptr;
        VkBuffer bvhBoxBuffer = nullptr;
        uint32_t entryID = 0, primitiveCount = -1, primitiveOffset = 0;
        uint32_t bvhMetaOffset = 0, bvhBoxOffset = 0;
        VkBool32 secondary = false; // used for copying and storing only?
    };

    // custom (unified) object create info, exclusive for vRt ray tracing system, and based on classic Satellite objects
    // bound in device space
    struct VtDeviceBufferCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_BUFFER_CREATE_INFO;
        const void* pNext = nullptr;

        VkBufferUsageFlags usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkDeviceSize bufferSize = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint32_t familyIndex = 0;
    };


    struct VtBufferRegionCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_BUFFER_REGION_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize offset = 0;
        VkDeviceSize bufferSize = sizeof(uint32_t);
        VkFormat format = VK_FORMAT_UNDEFINED;
    };


    struct VtDeviceImageCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_IMAGE_CREATE_INFO;
        const void* pNext = nullptr;

        VkImageViewType imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        VkImageLayout layout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
        VkExtent3D size = { 1, 1, 1 };
        VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkFormat format = VK_FORMAT_R32G32B32A32_UINT;
        uint32_t mipLevels = 1;
        uint32_t familyIndex = 0;
    };

};
