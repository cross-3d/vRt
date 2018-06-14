#pragma once

#include "../utils.hpp"

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_mem_alloc.h>
#include <vRt/vRt.h>

namespace NSM
{
    constexpr auto DEFAULT_FENCE_TIMEOUT = 100000000000ll;

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
        vk::Queue queue;
    };

    struct DeviceType : public std::enable_shared_from_this<DeviceType> {
        vt::VtDevice rtDev;
        vk::Device logical;
        vk::PhysicalDevice physical;

        vk::DescriptorPool descriptorPool;
        vk::PipelineCache pipelineCache;
        vk::DispatchLoaderDynamic dldid;
        VmaAllocator allocator;

        std::vector<DevQueue> queues;
        operator vk::Device() const { return logical; }
        operator vt::VtDevice() const { return rtDev; }
    };

    // application device structure
    struct QueueType : public std::enable_shared_from_this<QueueType> {
        Device device;
        vk::CommandPool commandPool;
        vk::Queue queue;
        vk::Fence fence;
        uint32_t familyIndex = 0;

        operator Device() const { return device; }
        operator vk::Device() const { return device->logical; }
        operator vt::VtDevice() const { return device->rtDev; }
        operator vk::Queue() const { return queue; }
    };

    // application surface format information structure
    struct SurfaceFormat : public std::enable_shared_from_this<SurfaceFormat> {
        vk::Format colorFormat;
        vk::Format depthFormat;
        vk::Format stencilFormat;
        vk::ColorSpaceKHR colorSpace;
        vk::FormatProperties colorFormatProperties;
    };

    // framebuffer with command buffer and fence
    struct Framebuffer : public std::enable_shared_from_this<Framebuffer> {
        vk::Framebuffer frameBuffer;
        vk::CommandBuffer commandBuffer; // terminal command (barrier)
        vk::Fence waitFence;
        vk::Semaphore semaphore;
    };

    // vertex layout
    struct VertexLayout : public std::enable_shared_from_this<VertexLayout> {
        std::vector<vk::VertexInputBindingDescription> inputBindings;
        std::vector<vk::VertexInputAttributeDescription> inputAttributes;
    };

    // context for rendering (can be switched)
    struct GraphicsContext : public std::enable_shared_from_this<GraphicsContext> {
        Queue queue;     // used device by context
        vk::SwapchainKHR swapchain; // swapchain state
        vk::Pipeline pipeline;      // current pipeline
        vk::PipelineLayout pipelineLayout;
        vk::PipelineCache pipelineCache;
        vk::DescriptorPool descriptorPool; // current descriptor pool
        vk::RenderPass renderpass;
        std::vector<vk::DescriptorSet> descriptorSets; // descriptor sets
        std::vector<Framebuffer> framebuffers;         // swapchain framebuffers
    };

    // compute context
    struct ComputeContextType : public std::enable_shared_from_this<ComputeContextType> {
        Queue queue;          // used device by context
        vk::CommandBuffer commandBuffer; // command buffer of compute context
        vk::Pipeline pipeline;           // current pipeline
        vk::PipelineCache pipelineCache;
        vk::PipelineLayout pipelineLayout;
        vk::Fence waitFence;             // wait fence of computing
        vk::DescriptorPool descriptorPool;             // current descriptor pool
        std::vector<vk::DescriptorSet> descriptorSets; // descriptor sets
    };
};
