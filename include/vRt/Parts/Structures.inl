#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"
#include "StructuresDef.inl"
#include "Enums.inl"
#include "Handlers.inl"

namespace vt { // store in official namespace
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
        size_t sharedCacheSize = 16u * 1024u * 1024u;
        size_t maxPrimitives = 1024u * 1024u;
        std::string_view shaderPath = "./";
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

        size_t maxRays = 4096u * 4096u;
    };

    struct VtRayTracingPipelineCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO;
        const void* pNext = nullptr;

        // ray tracing based pipeline layout
        VtPipelineLayout pipelineLayout;

        // TODO: entry points support (default = "main")
        // shader paths to ray tracing stages shaders 
        //std::string_view generationShader = "raytracing/generation-shader.comp.spv";
        //std::string_view closestShader = "raytracing/closest-hit-shader.comp.spv";
        //std::string_view missShader = "raytracing/miss-hit-shader.comp.spv";
        //std::string_view resolveShader = "raytracing/resolve-shader.comp.spv";

        VkPipelineShaderStageCreateInfo generationModule;
        VkPipelineShaderStageCreateInfo closestModule;
        VkPipelineShaderStageCreateInfo missModule;
        VkPipelineShaderStageCreateInfo resolveModule;
    };



    // use immutables in accelerator inputs
    // planned support of indirect const buffers
    struct VtVertexInputCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO;
        const void* pNext = nullptr;

        // all sources buffer
        VkBuffer sourceBuffer = nullptr;
        uint32_t primitiveCount = 0;
        //uint32_t sourceBufferBinding;

        // bindings regions
        VtVertexRegionBinding * pBufferRegionBindings = nullptr;
        VkBuffer bBufferRegionBindings = nullptr; // direct descriptor set bind
        uint32_t bufferRegionCount = 0;

        // accessor regions
        VtVertexAccessor * pBufferAccessors = nullptr;
        VkBuffer bBufferAccessors = nullptr; // direct descriptor set bind
        uint32_t bufferAccessorCount = 0;

        // attribute bindings (will stored in special patterned image buffer)
        VtVertexAttributeBinding * pBufferAttributeBindings = nullptr;
        VkBuffer bBufferAttributeBindings = nullptr; // direct descriptor set bind
        uint32_t attributeBindingCount = 0;
        uint32_t attributeByteOffset = 0; // for buffer maners

        VtVertexBufferView * pBufferViews = nullptr;
        VkBuffer bBufferViews = nullptr; // direct descriptor set bind
        uint32_t bufferViewCount = 0;

        // where from must got vertex and indices
        uint32_t verticeAccessor = 0;
        uint32_t indiceAccessor = 0xFFFFFFFF; // has no indice accessor

        // supported vertex topology
        VtTopologyType topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;
        uint32_t materialID = 0; // material ID for identify in hit shader
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
    struct VtVertexAssemblySetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ASSEMBLY_SET_CREATE_INFO;
        const void* pNext = nullptr;

        size_t maxPrimitives = 1024u * 1024u;
    };


    // 
    struct VtAcceleratorSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ACCELERATOR_SET_CREATE_INFO;
        const void* pNext = nullptr;

        size_t maxPrimitives = 1024u * 1024u;
        //VtVertexAssemblySet vertexAssembly;
    };



    // custom (unified) object create info, exclusive for vRt ray tracing system, and based on classic Satellite objects
    // bound in device space

    struct VtDeviceBufferCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_BUFFER_CREATE_INFO;
        const void* pNext = nullptr;

        VkBufferUsageFlags usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkDeviceSize bufferSize = sizeof(uint32_t);
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint32_t familyIndex = 0;
    };

    struct VtDeviceImageCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_IMAGE_CREATE_INFO;
        const void* pNext = nullptr;

        VkImageViewType imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        VkImageLayout layout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
        VkExtent3D size = { 1, 1, 1 };
        VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
        uint32_t mipLevels = 1;
        uint32_t familyIndex = 0;
    };


};
