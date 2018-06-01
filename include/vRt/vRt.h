#pragma once
#include <vulkan/volk.h>
#include <vulkan/vk_mem_alloc.h>
#include <memory>

namespace vt { // store in official namespace

    // for not confusing with Vulkan API
    // use 0x11E for VtResult
    // use 0x11F for VtStructureType

     enum VtResult: uint32_t {
        VT_SUCCESS = 0x11E00000, // default status
        VT_NOT_READY = 0x11E00001, 
        VT_TIMEOUT = 0x11E00002, 
        VT_INCOMPLETE = 0x11E00003, 
        VT_ERROR_INITIALIZATION_FAILED = 0x11E00004, // if error occurs from ray tracer itself
    };

     enum VtPipelineBindPoint: uint32_t {
        VT_PIPELINE_BIND_POINT_RAY_TRACING = 0x11F00000,
        VT_PIPELINE_BIND_POINT_ACCELERATOR = 0x11F00001 // unknown
    };

     enum VtStructureType: uint32_t {
        VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 0x11F00000,
        VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 0x11F00001,
        VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO = 0x11F00002, 
        VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO = 0x11F00003,
        VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO = 0x11F00004,
        VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO = 0x11F00005,
        VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO = 0x11F00006,
        //VT_STRUCTURE_TYPE_VERTEX_ACCESSOR = 0x11F00007,
        //VT_STRUCTURE_TYPE_VERTEX_ATTRIBUTE_BINDING = 0x11F00008,
        //VT_STRUCTURE_TYPE_VERTEX_REGION_BINDING = 0x11F00009,
        VT_STRUCTURE_TYPE_MATERIALS_INPUT_CREATE_SET_INFO = 0x11F0000A,
    };

    // it is bitfield-based value
     enum VtFormat: uint32_t {

    };

    // all supported topologies
     enum VtTopologyType: uint32_t {
        VT_TOPOLOGY_TYPE_TRIANGLES_LIST = 0x11F00000
    };

};


// C++ hard interfaces
namespace _vt { // store in undercover namespace
    class Instance;
    class PhysicalDevice;
    class Device;
    //class RayTracing;
    class CommandBuffer;
    class Pipeline;

    // acceleration and vertex input
    class VertexInput;
    class Accelerator;

    // materials (with textures) inputs
    class MaterialsInput;


    class Instance : public std::enable_shared_from_this<Instance> {
    protected:
        VkInstance _instance;
    public:
        operator VkInstance() const { return _instance; }
    };


    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    protected:
        friend Instance;
        std::shared_ptr<Instance> _instance;
        VkPhysicalDevice _physicalDevice;
    public:
        operator VkPhysicalDevice() const { return _physicalDevice; }
        std::shared_ptr<Instance> _parent() const { return _instance; };
    };


    class Device : public std::enable_shared_from_this<Device> {
    protected:
        friend PhysicalDevice;
        std::shared_ptr<PhysicalDevice> _physicalDevice;
        VkDevice _device;
    public:
        operator VkDevice() const { return _device; }
        std::shared_ptr<PhysicalDevice> _parent() const { return _physicalDevice; };
    };


    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
        VkCommandBuffer _cmd;
    public:
        operator VkCommandBuffer() const { return _cmd; }
        std::shared_ptr<Device> _parent() const { return _device; };
    };

    /*
    class RayTracing: public std::enable_shared_from_this<RayTracing> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
    public:
        //operator VkRayTracing() const {} // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };*/


    class Pipeline: public std::enable_shared_from_this<Pipeline> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
    public:
        //operator VkPipeline() const {} // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };


    class Accelerator: public std::enable_shared_from_this<Accelerator> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
    public:
        //operator VkAccelerator() const {} // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };
};


// class aliases for vRt from C++ hard implementators (incomplete)
// use shared pointers for C++
// (planned use plain pointers in C)
namespace vt { // store in official namespace

    struct VtInstance {
        std::shared_ptr<_vt::Instance> _vtInstance;
        operator VkInstance() const { return *_vtInstance; }
    };

    struct VtPhysicalDevice {
        std::shared_ptr<_vt::PhysicalDevice> _vtPhysicalDevice;
        operator VkPhysicalDevice() const { return *_vtPhysicalDevice; }
    };

    struct VtDevice {
        std::shared_ptr<_vt::Device> _vtDevice;
        operator VkDevice() const { return *_vtDevice; }
    };

    struct VtCommandBuffer {
        std::shared_ptr<_vt::CommandBuffer> _vtCommandBuffer;
        operator VkCommandBuffer() const { return *_vtCommandBuffer; }
    };

    struct VtPipeline {
        std::shared_ptr<_vt::Pipeline> _vtPipeline;
        
    };

};


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
        VtVertexAccessors * pBufferAccessors = nullptr;
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
        VtStructureType sType = VT_STRUCTURE_TYPE_MATERIALS_INPUT_CREATE_SET_INFO;
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
    VtResult vtCreateMaterialsInput(const VtMaterialsCreateInfo * vtMaterialsCreateInfo, VtMaterialsInput * materialsInput);


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