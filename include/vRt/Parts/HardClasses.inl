#pragma once
#include "Vulkan.inl"
#include "Enums.inl"

// C++ hard interfaces (which will storing)
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
    public:
        VkInstance _instance;
    //public:
        operator VkInstance() const { return _instance; }
    };


    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    public:
        friend Instance;
        std::shared_ptr<Instance> _instance;
        VkPhysicalDevice _physicalDevice;
    //public:
        operator VkPhysicalDevice() const { return _physicalDevice; }
        std::shared_ptr<Instance> _parent() const { return _instance; };
    };


    class Device : public std::enable_shared_from_this<Device> {
    public:
        friend PhysicalDevice;
        std::shared_ptr<PhysicalDevice> _physicalDevice;
        VkDevice _device;
    //public:
        operator VkDevice() const { return _device; }
        std::shared_ptr<PhysicalDevice> _parent() const { return _physicalDevice; };
    };


    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    public:
        friend Device;
        std::shared_ptr<Device> _device;
        VkCommandBuffer _cmd;
    //public:
        operator VkCommandBuffer() const { return _cmd; }
        std::shared_ptr<Device> _parent() const { return _device; };
    };

    /*
    class RayTracing: public std::enable_shared_from_this<RayTracing> {
    public:
        friend Device;
        std::shared_ptr<Device> _device;
    public:
        //operator VkRayTracing() const {} // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };*/


    class Pipeline: public std::enable_shared_from_this<Pipeline> {
    public:
        friend Device;
        std::shared_ptr<Device> _device;
    //public:
        //operator VkPipeline() const {} // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };


    class Accelerator: public std::enable_shared_from_this<Accelerator> {
    public:
        friend Device;
        std::shared_ptr<Device> _device;
    //public:
        //operator VkAccelerator() const {} // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };
};