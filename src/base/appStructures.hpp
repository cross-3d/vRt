#pragma once

#include "appUtils.hpp"

namespace NSM {
    using namespace _vt;

    struct QueueType;
    struct DevQueueType;
    struct DeviceType;
    struct QueueType;
    struct BufferType;
    struct ImageType;
    struct SamplerType;
    struct ImageCombinedType;
    struct ComputeContextType;

    using DevQueue = std::shared_ptr<DevQueueType>;
    using Device = std::shared_ptr<DeviceType>;
    using Queue = std::shared_ptr<QueueType>;
    using Buffer = std::shared_ptr<BufferType>;
    using Image = std::shared_ptr<ImageType>;
    using Sampler = std::shared_ptr<SamplerType>;
    using ImageCombined = std::shared_ptr<ImageCombinedType>;
    using ComputeContext = std::shared_ptr<ComputeContextType>;

    struct DevQueueType : public std::enable_shared_from_this<DevQueueType> {
        uint32_t familyIndex = 0;
        vk::Queue queue = nullptr;
    };

    struct DeviceType : public std::enable_shared_from_this<DeviceType> {
        vrt::VtDevice rtDev;
        vk::Device logical = nullptr;
        vk::PhysicalDevice physical = nullptr;

        vk::DescriptorPool descriptorPool = nullptr;
        vk::PipelineCache pipelineCache = nullptr;
        vk::DispatchLoaderDynamic dldid = {};
        VmaAllocator allocator;

        std::vector<DevQueue> queues;
        operator vk::Device() const { return logical; }
        operator vrt::VtDevice() const { return rtDev; }
    };

    // combined device, command pool and queue
    struct QueueType : public std::enable_shared_from_this<QueueType> {
        Device device;
        vk::CommandPool commandPool = nullptr;
        vk::Queue queue = nullptr;
        vk::Fence fence = nullptr;
        uint32_t familyIndex = 0;

        operator Device() const { return device; }
        operator vk::Device() const { return device->logical; }
        operator vrt::VtDevice() const { return device->rtDev; }
        operator vk::Queue() const { return queue; }
    };

    // application surface format information structure
    //struct SurfaceFormat : public std::enable_shared_from_this<SurfaceFormat> {
    struct SurfaceFormat {
        vk::Format colorFormat = {};
        vk::Format depthFormat = {};
        vk::Format stencilFormat = {};
        vk::ColorSpaceKHR colorSpace = {};
        vk::FormatProperties colorFormatProperties = {};
    };

    // framebuffer with command buffer and fence
    struct Framebuffer : public std::enable_shared_from_this<Framebuffer> {
        vk::Framebuffer frameBuffer = nullptr;
        vk::CommandBuffer commandBuffer = nullptr; // terminal command (barrier)
        vk::Fence waitFence = nullptr;
        vk::Semaphore semaphore = nullptr;
    };

    // vertex layout
    struct VertexLayout : public std::enable_shared_from_this<VertexLayout> {
        std::vector<vk::VertexInputBindingDescription> inputBindings;
        std::vector<vk::VertexInputAttributeDescription> inputAttributes;
    };

    // context for rendering (can be switched)
    struct GraphicsContext : public std::enable_shared_from_this<GraphicsContext> {
        Queue queue;                                   // used device by context
        vk::SwapchainKHR swapchain = nullptr;          // swapchain state
        vk::Pipeline pipeline = nullptr;               // current pipeline
        vk::PipelineLayout pipelineLayout = nullptr;
        vk::PipelineCache pipelineCache = nullptr;
        vk::DescriptorPool descriptorPool = nullptr;   // current descriptor pool
        vk::RenderPass renderpass = nullptr;
        std::vector<vk::DescriptorSet> descriptorSets; // descriptor sets
        std::vector<Framebuffer> framebuffers;         // swapchain framebuffers
    };
};
