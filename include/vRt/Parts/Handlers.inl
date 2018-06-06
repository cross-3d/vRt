#pragma once
#include "Headers.inl"
#include "HardClasses.inl"

// class aliases for vRt from C++ hard implementators (incomplete)
// use shared pointers for C++
// (planned use plain pointers in C)
namespace vt { // store in official namespace

    struct VtInstance {
        std::shared_ptr<_vt::Instance> _vtInstance;
        operator VkInstance() const { return *_vtInstance; }
        operator bool() const { return !!_vtInstance; };
    };

    struct VtPhysicalDevice {
        std::shared_ptr<_vt::PhysicalDevice> _vtPhysicalDevice;
        operator VkPhysicalDevice() const { return *_vtPhysicalDevice; }
        operator bool() const { return !!_vtPhysicalDevice; };
    };

    struct VtDevice {
        std::shared_ptr<_vt::Device> _vtDevice;
        operator VkDevice() const { return *_vtDevice; }
        operator bool() const { return !!_vtDevice; };
    };

    struct VtCommandBuffer {
        std::shared_ptr<_vt::CommandBuffer> _vtCommandBuffer;
        operator VkCommandBuffer() const { return *_vtCommandBuffer; }
        operator bool() const { return !!_vtCommandBuffer; };
    };

    struct VtPipelineLayout {
        std::shared_ptr<_vt::PipelineLayout> _vtPipelineLayout;
        operator VkPipelineLayout() const { return *_vtPipelineLayout; }
        operator bool() const { return !!_vtPipelineLayout; };
    };

    struct VtPipeline {
        std::shared_ptr<_vt::Pipeline> _vtPipeline;
        operator bool() const { return !!_vtPipeline; }
    };

    struct VtAccelerator {
        std::shared_ptr<_vt::Accelerator> _vtAccelerator;
        operator bool() const { return !!_vtAccelerator; }
    };

    struct VtMaterialSet {
        std::shared_ptr<_vt::MaterialSet> _vtMaterialSet;
        operator VkDescriptorSet() const { return *_vtMaterialSet; }
        operator bool() const { return !!_vtMaterialSet; }
    };

    // advanced class (buffer)
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY>
    struct VtRoledBuffer {
        std::shared_ptr<_vt::RoledBuffer<U>> _vtBuffer;
        operator VkBuffer() const { return *_vtBuffer; }
        operator VkBufferView() const { return *_vtBuffer; }
        operator bool() const { return !!_vtBuffer; }
    };

    // advanced class (image)
    struct VtDeviceImage {
        std::shared_ptr<_vt::DeviceImage> _vtDeviceImage;
        operator VkImage() const { return *_vtDeviceImage; }
        operator VkImageView() const { return *_vtDeviceImage; }
        operator bool() const { return !!_vtDeviceImage; }
    };

    // aliases
    using VtDeviceBuffer = VtRoledBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    using VtHostToDeviceBuffer = VtRoledBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    using VtDeviceToHostBuffer = VtRoledBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;

};