#pragma once
#include "Vulkan.inl"
#include "HardClasses.inl"

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

    struct VtPipelineLayout {
        std::shared_ptr<_vt::PipelineLayout> _vtPipelineLayout;
        operator VkPipelineLayout() const { return *_vtPipelineLayout; }
    };

    struct VtPipeline {
        std::shared_ptr<_vt::Pipeline> _vtPipeline;
        
    };

    struct VtAccelerator {
        std::shared_ptr<_vt::Accelerator> _vtPipeline;

    };

    struct VtMaterialsInput {
        std::shared_ptr<_vt::MaterialsInput> _vtMaterialsInput;

    };

    struct VtVertexInput {
        std::shared_ptr<_vt::MaterialsInput> _vtVertexInput;

    };

};