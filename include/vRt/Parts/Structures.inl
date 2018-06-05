#pragma once
#include "Headers.inl"
#include "Enums.inl"

namespace vt { // store in official namespace
    // Description structs for make vRt objects
    // Note: structures already have default headers for identifying


    // in general that is conversion
    struct VtInstanceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_INSTANCE_CONVERSION_INFO;
        const void* pNext = nullptr;
        //VkInstance vkInstance;
    };

    struct VtArtificalDeviceExtension {
        VtStructureType sType = VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION;
        const void* pNext = nullptr;
        // TODO to complete
    };

    /*
    struct VtRayTracingCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO;
        const void* pNext = nullptr;
        VtDevice vtDevice;
    };*/

    struct VtDeviceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CONVERSION_INFO;
        const void* pNext = nullptr;
        VtPhysicalDevice physicalDevice;
        //VkDevice vkDevice;
    };

    struct VtPhysicalDeviceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERSION_INFO;
        const void* pNext = nullptr;
        VtInstance instance;
        //VkPhysicalDevice vkPhysicalDevice;
    };

    struct VtRayTracingPipelineCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO;
        const void* pNext = nullptr;
        //VtDevice vtDevice;
    };

    // any other vertex accessors can be used by attributes
    struct VtVertexAccessor {
        uint32_t bufferViewID = 0;
        uint32_t byteOffset = 0;
        union {
            uint32_t components : 2, type : 4, normalized : 1;
            VtFormat format;
        };
    };

    // any other vertex bindings can be used by attributes
    struct VtVertexRegionBinding {
        uint32_t byteOffset = 0;
        uint32_t byteSize = 0;
    };

    // buffer view
    struct VtVertexBufferView {
        uint32_t regionID = 0;
        uint32_t byteOffset = 0;
        uint32_t byteStride = 0;
    };

    // attribute binding
    struct VtVertexAttributeBinding {
        uint32_t attributeBinding = 0;
        uint32_t accessorID = 0;
    };

    // use immutables in accelerator inputs
    struct VtVertexInputCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO;
        const void* pNext = nullptr;

        // all sources buffer
        //VkBuffer sourceBuffer;
        uint32_t sourceBufferBinding;

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

    // use as low level typed descriptor set
    struct VtMaterialSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_MATERIAL_SET_CREATE_INFO;
        const void* pNext = nullptr;

        // immutable images (textures)
        const VkDescriptorImageInfo* pImages = nullptr;
        uint32_t imageCount;

        // immutable samplers
        const VkSampler* pSamplers = nullptr;
        uint32_t samplerCount;

        // virtual combined textures with samplers (better use in buffers)
        const uint64_t* pImageSamplerCombinations = nullptr; // uint32 for images and next uint32 for sampler ID's
        uint32_t imageSamplerCount;

        // user defined materials 
        VkBuffer materialDescriptionsBuffer;
        //const uint32_t* pMaterialDescriptionIDs = nullptr; // I don't remember why it need
        uint32_t materialCount;
    };

    struct VtAcceleratorCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ACCELERATOR_CREATE_INFO;
        const void* pNext = nullptr;

        // prefer to describe vertex input in accelerator for creation (and just more safer)
        const VtVertexInputCreateInfo * pVertexInputs = nullptr;
        uint32_t vertexInputCount = 0;
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


    struct VtVirtualCombinedImage {
        union {
            uint64_t textureID : 32, samplerID : 32;
            uint64_t combined;
        };

        operator uint64_t() const { return combined; };
        operator uint64_t&() { return combined; };
    };



    // custom (unified) object create info, exclusive for vRt ray tracing system, and based on classic Satellite objects
    // bound in device space
    // planned to add support of HostToGpu and GpuToHost objects 

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
        VkExtent3D size = {1, 1, 1};
        VkImageUsageFlags usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
        uint32_t mipLevels = 1;
        uint32_t familyIndex = 0;
    };

};
