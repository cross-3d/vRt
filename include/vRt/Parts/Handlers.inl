#pragma once
#include "Headers.inl"
#include "HandlersDef.inl"
#include "HardClassesDef.inl"
#include "HardClasses.inl"

// class aliases for vRt from C++ hard implementators (incomplete)
// use shared pointers for C++
// (planned use plain pointers in C)
namespace vt { // store in official namespace

    struct VtInstance {
        std::shared_ptr<_vt::Instance> _vtInstance;
        operator std::shared_ptr<_vt::Instance>() const { return _vtInstance; };
        operator std::shared_ptr<_vt::Instance>&() { return _vtInstance; };
        operator VkInstance() const { return *_vtInstance; };
        operator bool() const { return !!_vtInstance; };
    };

    struct VtPhysicalDevice {
        std::shared_ptr<_vt::PhysicalDevice> _vtPhysicalDevice;
        operator std::shared_ptr<_vt::PhysicalDevice>() const { return _vtPhysicalDevice; };
        operator std::shared_ptr<_vt::PhysicalDevice>&() { return _vtPhysicalDevice; };
        operator VkPhysicalDevice() const { return *_vtPhysicalDevice; };
        operator bool() const { return !!_vtPhysicalDevice; };
    };

    struct VtDevice {
        std::shared_ptr<_vt::Device> _vtDevice;
        operator std::shared_ptr<_vt::Device>() const { return _vtDevice; };
        operator std::shared_ptr<_vt::Device>&() { return _vtDevice; };
        operator VkDevice() const { return *_vtDevice; }
        operator VkPipelineCache() const { return *_vtDevice; };
        operator VkDescriptorPool() const { return *_vtDevice; };
        operator VmaAllocator() const { return *_vtDevice; }
        operator bool() const { return !!_vtDevice; };
    };

    struct VtCommandBuffer {
        std::shared_ptr<_vt::CommandBuffer> _vtCommandBuffer;
        operator std::shared_ptr<_vt::CommandBuffer>() const { return _vtCommandBuffer; };
        operator std::shared_ptr<_vt::CommandBuffer>&() { return _vtCommandBuffer; };
        operator VkCommandBuffer() const { return *_vtCommandBuffer; };
        operator bool() const { return !!_vtCommandBuffer; };
    };

    struct VtPipelineLayout {
        std::shared_ptr<_vt::PipelineLayout> _vtPipelineLayout;
        operator std::shared_ptr<_vt::PipelineLayout>() const { return _vtPipelineLayout; };
        operator std::shared_ptr<_vt::PipelineLayout>&() { return _vtPipelineLayout; };
        operator VkPipelineLayout() const { return *_vtPipelineLayout; };
        operator bool() const { return !!_vtPipelineLayout; };
    };

    struct VtPipeline {
        std::shared_ptr<_vt::Pipeline> _vtPipeline;
        operator std::shared_ptr<_vt::Pipeline>() const { return _vtPipeline; };
        operator std::shared_ptr<_vt::Pipeline>&() { return _vtPipeline; };
        operator bool() const { return !!_vtPipeline; };
    };

    struct VtAccelerator {
        std::shared_ptr<_vt::Accelerator> _vtAccelerator;
        operator std::shared_ptr<_vt::Accelerator>() const { return _vtAccelerator; };
        operator std::shared_ptr<_vt::Accelerator>&() { return _vtAccelerator; };
        operator bool() const { return !!_vtAccelerator; };
    };

    struct VtMaterialSet {
        std::shared_ptr<_vt::MaterialSet> _vtMaterialSet;
        operator std::shared_ptr<_vt::MaterialSet>() const { return _vtMaterialSet; };
        operator std::shared_ptr<_vt::MaterialSet>&() { return _vtMaterialSet; };
        operator VkDescriptorSet() const { return *_vtMaterialSet; };
        operator bool() const { return !!_vtMaterialSet; };
    };

    // advanced class (buffer)
    template<VmaMemoryUsage U>
    struct VtRoledBuffer {
        std::shared_ptr<_vt::RoledBuffer<U>> _vtBuffer;
        operator std::shared_ptr<_vt::RoledBuffer<U>>() const { return _vtBuffer; };
        operator std::shared_ptr<_vt::RoledBuffer<U>>&() { return _vtBuffer; };
        operator VkBuffer() const { return *_vtBuffer; };
        operator VkBufferView() const { return *_vtBuffer; };
        operator bool() const { return !!_vtBuffer; };
    };

    // advanced class (image)
    struct VtDeviceImage {
        std::shared_ptr<_vt::DeviceImage> _vtDeviceImage;
        operator std::shared_ptr<_vt::DeviceImage>() const { return _vtDeviceImage; };
        operator std::shared_ptr<_vt::DeviceImage>&() { return _vtDeviceImage; };
        operator VkImage() const { return *_vtDeviceImage; };
        operator VkImageView() const { return *_vtDeviceImage; };
        operator bool() const { return !!_vtDeviceImage; };
    };

};