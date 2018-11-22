#pragma once

// implementable
#include "Handlers.inl"

namespace vrt { // store in official namespace
    // Description structs for make vRt objects
    // Note: structures already have default headers for identifying


    // in general that is conversion
    struct VtInstanceConversionInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_INSTANCE_CONVERSION_INFO;
        const void* pNext = nullptr;
    };


    // should be implemented support for enabling
    // planned connection with VtAcceleratorName, and automatic initialization
    // passing with pNext to VtDeviceAggregationInfo
    class VtDeviceAdvancedAccelerationExtension {
    public: // regular API chaining
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_ADVANCED_ACCELERATION_EXTENSION;
        const void* pNext = nullptr;

    public: // API in-runtime implementation (dynamic polymorphism)
        virtual VtAccelerationName _AccelerationName() const { return VT_ACCELERATION_NAME_UNKNOWN; }; // in-runtime return acceleration extension name
        virtual VtResult _Criteria(std::shared_ptr<_vt::DeviceFeatures> lwFeatures) const { return VK_ERROR_EXTENSION_NOT_PRESENT; };
        virtual VtResult _Initialization(std::shared_ptr<_vt::Device> lwDevice, std::shared_ptr<_vt::AcceleratorExtensionBase>& _hExtensionAccelerator) const { return VK_ERROR_EXTENSION_NOT_PRESENT; };

    public: // can have custom data...

    };


    struct VtDeviceAggregationInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_AGGREGATION_INFO;
        const void* pNext = nullptr;
        
        uint32_t familyIndiceCount = 0u;
        const uint32_t* pFamilyIndices = nullptr;

        VkDeviceSize sharedCacheSize = 64ull * 1024ull;
        VkDeviceSize maxPrimitives = 1024ull * 1024ull;
        
#ifdef VRT_ENABLE_STRING_VIEW
        std::string_view shaderPath = "./intrusive";
#else
        std::string shaderPath = "./intrusive";
#endif
#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
        const void* allocator = nullptr;
#else
        const VmaAllocator* allocator = nullptr;
#endif

        VkBool32 enableAdvancedAcceleration = false; // such as RTX, in current moment have no any support 
        const VtDeviceAdvancedAccelerationExtension* pAccelerationExtension = nullptr; // inline structure isn't acceptable 
    };


    //struct VtDeviceConversionInfo {
    //    VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CONVERSION_INFO;
    //    const void* pNext = nullptr;
    //    VtPhysicalDevice physicalDevice;
    //};


    //struct VtPhysicalDeviceConversionInfo {
    //    VtStructureType sType = VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERSION_INFO;
    //    const void* pNext = nullptr;
    //    VtInstance instance;
    //};


    struct VtRayTracingSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_RAY_TRACING_SET_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize maxRays = 1920ull * 1200ull, maxHits = 0ull;
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
        VkExtent2D tiling = {8u, 8u};
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

        VtAttributePipeline attributeAssembly = {};
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
        //const VkBufferView * pSourceBuffers = nullptr;
        const VtBufferRegion * pSourceBuffers = nullptr;
        uint32_t sourceBufferCount = 0;

        // bindings regions
        //VkBuffer bBufferRegionBindings = nullptr; // direct descriptor set bind
        //VkDeviceSize bufferRegionByteOffset = 0;

        // accessor regions
        VkBuffer bBufferAccessors = nullptr; // direct descriptor set bind
        VkDeviceSize bufferAccessorByteOffset = 0;

        // attribute bindings (will stored in special patterned image buffer)
        VkBuffer bBufferAttributeBindings = nullptr; // direct descriptor set bind
        VkDeviceSize attributeByteOffset = 0; // for buffer maners

        // buffer views
        VkBuffer bBufferViews = nullptr; // direct descriptor set bind
        VkDeviceSize bufferViewByteOffset = 0;

        // 
        VkBuffer bTransformData = nullptr;
        VkDeviceSize transformOffset = 0;

        // vertex accessor (required for direct mode on RTX-like raytracers)
        // buffer-based accessors also can be used, but only in vertex assembly middleware mode (i.e. eqaulize as single geometry) 
        // also, planned partitions of vertex assembly buffer for enable transformation support (i.e. fake multiple geometry mode) 
        VkBool32 directModeSupport = false; // may help to provide faster acceleration 
        const VtVertexAccessor* pVertexAccessor = nullptr;
        const VtVertexAccessor* pIndiceAccessor = nullptr; // WARNING: RTX does not support striding in direct mode
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
        const void* pImageSamplerCombinations = nullptr; // uint32 for images and next uint32 for sampler ID's 
        VkBuffer bImageSamplerCombinations = nullptr;
        uint32_t imageSamplerCount = 0;

        // user defined materials 
        VkBuffer bMaterialDescriptionsBuffer = nullptr;
        //const uint32_t* pMaterialDescriptionIDs = nullptr; // I don't remember why it need
        uint32_t materialCount = 0;
    };

    // 
    struct VtAttributePipelineCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ATTRIBUTE_PIPELINE_CREATE_INFO;
        const void* pNext = nullptr;

        //size_t maxPrimitives = 1024u * 256u;
        VtPipelineLayout pipelineLayout = {};
        VkPipelineShaderStageCreateInfo assemblyModule = {}, interpolModule = {};
    };

    // planned to deprecate 
    struct VtVertexAssemblySetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ASSEMBLY_SET_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize maxPrimitives = 1024ull * 64ull;

        // added in 10.10.2018 

        VtAttributePipeline vertexAssembly = {};
        VkBuffer sharedVertexCacheBuffer = VK_NULL_HANDLE; VkDeviceSize sharedVertexCacheOffset = 0ull; // before building BVH, will stored into 
        VkBuffer sharedVertexInUseBuffer = VK_NULL_HANDLE; VkDeviceSize sharedVertexInUseOffset = 0ull; // will converted to traversable format 
        VkBuffer sharedBitfieldBuffer = VK_NULL_HANDLE; VkDeviceSize sharedBitfieldOffset = 0ull; // bitfields data buffer with pointing offset 
        VkBuffer sharedMaterialIndexedBuffer = VK_NULL_HANDLE; VkDeviceSize sharedMaterialIndexedOffset = 0ull; // material indices buffer with pointing offset 
        VkBuffer sharedNormalBuffer = VK_NULL_HANDLE; VkDeviceSize sharedNormalOffset = 0ull; // normals buffer with pointing offset 
        VkDescriptorImageInfo sharedAttributeImageDescriptor = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL }; // sharing image by descriptor 
    };

    // 
    struct VtAcceleratorSetCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ACCELERATOR_SET_CREATE_INFO;
        const void* pNext = nullptr;

        // passing a simple meta data for builders
        VtAcceleratorSetLevel structureLevel = VT_ACCELERATOR_SET_LEVEL_GEOMETRY;
        VkDeviceSize maxPrimitives = 1024ull * 64ull; VkBool32 secondary = false; // used for copying and storing only?

        // linking with shared space of vertex assembly 
        VkDeviceSize vertexPointingOffset = 0ull;

        // entry point ID for traversing (required when using buffer sharing)
        // from bytes, measuring by 'BvhDataByteOffset/sizeof(VtBvhNodeStruct)'
        // note, entry point should indexing by Even i.e. powering by 2, so should have correction aligment (let's have small space, but correct aligment) 
        uint32_t traversingEntryID = 0ull;

        // dedicated buffers for reusing 
        VkBuffer bvhDataBuffer = VK_NULL_HANDLE; VkDeviceSize bvhDataOffset = 0ull; // external BVH data buffer 
        VkBuffer bvhHeadBuffer = VK_NULL_HANDLE; VkDeviceSize bvhHeadOffset = 0ull; // buffer, used in top levels 

        //VkBuffer bvhMetaHeadBuffer = VK_NULL_HANDLE; VkDeviceSize bvhMetaHeadOffset = 0ull; // buffer, used as header 
        VkBuffer bvhInstanceBuffer = VK_NULL_HANDLE; VkDeviceSize bvhInstanceOffset = 0ull; // will used for top levels 

        // 
        const VtAcceleratorSet * pStructVariations = nullptr;
        uint32_t structVariationCount = 0u;

        // mat4 of optimization
        VtMat4 coverMat = vrt::IdentifyMat4;
    };

    // custom (unified) object create info, exclusive for vRt ray tracing system, and based on classic Satellite objects
    // bound in device space
    struct VtDeviceBufferCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_BUFFER_CREATE_INFO;
        const void* pNext = nullptr;

        VkBufferUsageFlags usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkDeviceSize bufferSize = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };


    struct VtBufferRegionCreateInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_BUFFER_REGION_CREATE_INFO;
        const void* pNext = nullptr;

        VkDeviceSize offset = 0, bufferSize = sizeof(uint32_t);
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
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageCreateFlags flags = {};

        uint32_t mipLevels = 1;
    };


    struct VtAcceleratorBuildInfo {
        VtStructureType sType = VT_STRUCTURE_TYPE_ACCELERATOR_BUILD_INFO;
        const void* pNext = nullptr;

        uint32_t flags = 0u; // reserved for future usage 
        VkDeviceSize elementOffset = 0, elementSize = VK_WHOLE_SIZE;
    };


};
