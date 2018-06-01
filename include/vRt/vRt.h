#pragma once
#include <vulkan/volk.h>
#include <vulkan/vk_mem_alloc.h>
#include <memory>

typedef enum VtResult {
    VT_SUCCESS = 0, // default status
    VT_NOT_READY = 1, 
    VT_TIMEOUT = 2, 
    VT_INCOMPLETE = 3, 
    VT_ERROR_INITIALIZATION_FAILED = 4, // if error occurs from ray tracer itself
};

typedef enum VtStructureType {
    VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 0,
    VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 1,
    VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO = 2, 
    VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO = 3,
};


namespace _vt {
    class Instance;
    class PhysicalDevice;
    class Device;
    class RayTracing;
    class CommandBuffer;

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
};


// class aliases for vRt from C++ hard implementators
typedef std::shared_ptr<_vt::Instance> VtInstance;
typedef std::shared_ptr<_vt::PhysicalDevice> VtPhysicalDevice;
typedef std::shared_ptr<_vt::Device> VtDevice;
typedef std::shared_ptr<_vt::RayTracing> VtRayTracing;


// description structs for make vRt objects

struct VtInstanceCreateInfo {
    VtStructureType sType = VT_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    const void* pNext = nullptr;
};

struct VtDeviceCreateInfo {
    VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    const void* pNext = nullptr;
};

struct VtRayTracingCreateInfo {
    VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    const void* pNext = nullptr;
};

struct VtDeviceConvertInfo {
    VtStructureType sType = VT_STRUCTURE_TYPE_DEVICE_CONVERT_INFO;
    const void* pNext = nullptr;
};

struct VtPhysicalDeviceConvertInfo {
    VtStructureType sType = VT_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONVERT_INFO;
    const void* pNext = nullptr;
};



// wrapped API for create device, instance and physical device
VtResult vtCreateInstance(VkInstance vkInstance, const VtInstanceCreateInfo * vtInstanceCreateInfo, VtInstance * vtInstance);
VtResult vtEnumeratePhysicalDevices(VtInstance vtInstance, uint32_t* pPhysicalDeviceCount, VtPhysicalDevice* pPhysicalDevices);
VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VtDeviceCreateInfo * vtDeviceCreateInfo, VtDevice * vtDevice);

// conversion API objects from VK
VtResult vtConvertPhysicalDevice(VtInstance vtInstance, VkPhysicalDevice vkPhysicalDevice, const VtPhysicalDeviceConvertInfo * vtPhysicalDeviceConvertInfo, VtPhysicalDevice * vtPhysicalDevice);
VtResult vtConvertDevice(VtPhysicalDevice vtPhysicalDevice, VkDevice vkDevice, const VtDeviceConvertInfo * vtDeviceConvertInfo, VtDevice * vtDevice);

// create ray tracing instance
VtResult vtCreateRayTracing(VtDevice vtDevice, const VtRayTracingCreateInfo * vtRayTracingCreateInfo, VtRayTracing * vtRayTracing);

// use compute capable copy buffer
VtResult vtCmdCopyBuffer(VtCommandBuffer commandBuffer, VkBuffer src, VkBuffer dst, uint32_t regionCount, const VkBufferCopy* pRegions);

// make command buffer capable with ray tracing factory (VtCommandBuffer)
VtResult vtCmdQueryInterface(VkCommandBuffer commandBuffer, VtDevice device, VtCommandBuffer * vtCommandBuffer);

