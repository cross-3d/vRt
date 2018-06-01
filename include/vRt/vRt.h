#pragma once
#include <vulkan/volk.h>
#include <vulkan/vk_mem_alloc.h>
#include <memory>

// for not confusing with Vulkan API
// use 0x11E for VtResult
// use 0x11F for VtStructureType

typedef enum VtResult {
    VT_SUCCESS = 0x11E00000, // default status
    VT_NOT_READY = 0x11E00001, 
    VT_TIMEOUT = 0x11E00002, 
    VT_INCOMPLETE = 0x11E00003, 
    VT_ERROR_INITIALIZATION_FAILED = 0x11E00004, // if error occurs from ray tracer itself
};

typedef enum VtPipelineBindPoint {
    VT_PIPELINE_BIND_POINT_RAY_TRACING = 0x11F00000,
    VT_PIPELINE_BIND_POINT_ACCELERATOR = 0x11F00001 // unknown
};

typedef enum VtStructureType {
    VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 0x11F00000,
    VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 0x11F00001,
    VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO = 0x11F00002, 
    VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO = 0x11F00003,
    VT_STRUCTURE_TYPE_RAY_TRACING_CREATE_INFO = 0x11F00004,
    VT_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO = 0x11F00005,
    VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO = 0x11F00006,
    VT_STRUCTURE_TYPE_VERTEX_ACCESSOR = 0x11F00007,
    VT_STRUCTURE_TYPE_VERTEX_ATTRIBUTE_BINDING = 0x11F00008,
    VT_STRUCTURE_TYPE_VERTEX_REGION_BINDING = 0x11F00009
};

// it is bitfield-based value
typedef enum VtFormat {

};

// all supported topologies
typedef enum VtTopologyType {
    VT_TOPOLOGY_TYPE_TRIANGLES_LIST = 0x11F00000
};


namespace _vt {
    class Instance;
    class PhysicalDevice;
    class Device;
    class RayTracing;
    class CommandBuffer;
    class Pipeline;

    // acceleration and vertex input
    class VertexInput;
    class Accelerator;


    class Instance : public std::enable_shared_from_this<Instance> {
    protected:
        VkInstance _instance;
    public:
        operator VkInstance(){
            return _instance;
        }
    };


    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    protected:
        friend Instance;
        std::shared_ptr<Instance> _instance;
        VkPhysicalDevice _physicalDevice;
    public:
        operator VkPhysicalDevice(){
            return _physicalDevice;
        }
    };


    class Device : public std::enable_shared_from_this<Device> {
    protected:
        friend PhysicalDevice;
        std::shared_ptr<PhysicalDevice> _physicalDevice;
        VkDevice _device;
    public:
        operator VkDevice(){
            return _device;
        }
    };


    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
        VkCommandBuffer _cmd;
    public:
        operator VkCommandBuffer(){
            return _cmd;
        }
    };


    class RayTracing: public std::enable_shared_from_this<RayTracing> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
    public:
        //operator VkRayTracing() {} // no correct conversion
    };


    class Pipeline: public std::enable_shared_from_this<RayTracing> {
    protected:
        friend Device;
        std::shared_ptr<Device> _device;
    public:
        //operator VkPipeline() {} // no correct conversion
    };
};


// class aliases for vRt from C++ hard implementators (incomplete)
typedef std::shared_ptr<_vt::Instance> VtInstance;
typedef std::shared_ptr<_vt::PhysicalDevice> VtPhysicalDevice;
typedef std::shared_ptr<_vt::Device> VtDevice;
typedef std::shared_ptr<_vt::RayTracing> VtRayTracing;
typedef std::shared_ptr<_vt::Pipeline> VtPipeline;

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
    VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ACCESSOR;
    const void* pNext = nullptr;
    uint32_t binding = 0;
    int32_t byteOffset = 0;
    union {
        uint32_t components : 2, type : 4, normalized : 1;
        VtFormat format;
    };
};


// any other vertex bindings can be used by attributes
struct VtVertexRegionBinding {
    VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_REGION_BINDING;
    const void* pNext = nullptr;
    uint32_t binding = 0;
    uint32_t byteStride = 0;
    VkBufferCopy bufferRegion; // import from Vulkan
};


// attribute binding
struct VtVertexAttributeBinding {
    VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_ATTRIBUTE_BINDING;
    const void* pNext = nullptr;
    uint32_t attributeBinding = 0;
    uint32_t accessorID = 0;
};


struct VtVertexInputCreateInfo {
    VtStructureType sType = VT_STRUCTURE_TYPE_VERTEX_INPUT_CREATE_INFO;
    const void* pNext = nullptr;

    // all vertex sources buffer
    VkBuffer sourceBuffer;

    // bindings regions
    VtVertexRegionBinding * pBufferRegionBindings = nullptr;
    uint32_t bufferRegionCount = 0;

    // accessor regions
    VtVertexAccessors * pBufferAccessors = nullptr;
    uint32_t bufferAccessorCount = 0;

    // attribute bindings (will stored in special patterned image buffer)
    VtVertexAttributeBinding * pBufferAttributeBindings = nullptr;
    uint32_t attributeBindingCount = 0;

    // where from must got vertex and indices
    uint32_t verticeAccessor = 0;
    uint32_t indiceAccessor = 0xFFFFFFFF; // has no indice accessor

    // supported vertex topology
    VtTopologyType topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;
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

// build accelerator structure command
VtResult vtCmdBuildAccelerator(VtCommandBuffer commandBuffer /*,  */);

// descriptorSet = "0" will blocked by ray tracing system
VtResult vtCmdBindDescriptorSets(VtCommandBuffer commandBuffer, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);
