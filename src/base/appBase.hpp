#pragma once

//#ifdef OS_WIN
#if (defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER_) || defined(__MINGW64__)) 
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

//#ifdef OS_LNX
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#define RVT_IMPLEMENTATION
#include "appStructures.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace NSM
{
    using namespace vt;

    class ApplicationBase : public std::enable_shared_from_this<ApplicationBase> {
    protected:


        // application binding
        vk::Instance instance;

        // cached Vulkan API data
        std::vector<Queue> devices;

        // instance extensions
        std::vector<const char *> wantedExtensions = {
            "VK_KHR_get_physical_device_properties2",
            "VK_EXT_debug_report"
        };

        // default device extensions
        std::vector<const char *> wantedDeviceExtensions = {
            "VK_EXT_swapchain_colorspace",
            "VK_EXT_direct_mode_display",
            "VK_EXT_debug_marker",
            "VK_EXT_debug_report",
            "VK_EXT_sample_locations",
            "VK_EXT_shader_subgroup_vote",
            "VK_EXT_shader_subgroup_ballot",
            "VK_EXT_external_memory_host",
            "VK_EXT_conservative_rasterization",
            "VK_EXT_hdr_metadata",
            "VK_EXT_queue_family_foreign",
            "VK_EXT_sampler_filter_minmax",

            "VK_KHR_8bit_storage",
            "VK_KHR_16bit_storage",
            "VK_AMD_gpu_shader_int16",
            "VK_AMD_gpu_shader_half_float",

            "VK_AMD_buffer_marker",
            "VK_AMD_shader_info",
            "VK_AMD_shader_ballot",
            "VK_AMD_texture_gather_bias_lod",
            "VK_AMD_shader_image_load_store_lod",
            "VK_AMD_gcn_shader",
            "VK_AMD_shader_trinary_minmax",
            "VK_AMD_draw_indirect_count",

            "VK_KHR_descriptor_update_template",
            "VK_KHR_dedicated_allocation",
            "VK_KHR_incremental_present",
            "VK_KHR_push_descriptor",
            "VK_KHR_swapchain",
            "VK_KHR_sampler_ycbcr_conversion",
            "VK_KHR_image_format_list",
            "VK_KHR_sampler_mirror_clamp_to_edge",
            "VK_KHR_shader_draw_parameters",
            "VK_KHR_storage_buffer_storage_class",
            "VK_KHR_variable_pointers",
            "VK_KHR_relaxed_block_layout",
            "VK_KHR_display"
            "VK_KHR_display_swapchain"


            "VK_KHR_get_memory_requirements2",
            "VK_KHR_get_physical_device_properties2",
            "VK_KHR_get_surface_capabilities2",
            "VK_KHR_bind_memory2",
            "VK_KHR_maintenance1",
            "VK_KHR_maintenance2",
            "VK_KHR_maintenance3"
        };

        // instance layers
        std::vector<const char *> wantedLayers = {
            "VK_LAYER_LUNARG_standard_validation",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_core_validation",
            //"VK_LAYER_RENDERDOC_Capture",
            //"VK_LAYER_LUNARG_assistant_layer",
            //"VK_LAYER_LUNARG_vktrace",
            //"VK_LAYER_GOOGLE_threading"
        };

        // default device layers
        std::vector<const char *> wantedDeviceValidationLayers = {
            "VK_LAYER_AMD_switchable_graphics"
        };


        struct SurfaceWindow {
            SurfaceFormat surfaceFormat;
            vk::Extent2D surfaceSize;
            vk::SurfaceKHR surface;
            GLFWwindow *window;
        } applicationWindow;

    public:

        vk::Instance createInstance() {
            volkInitialize();
            
            // get required extensions
            unsigned int glfwExtensionCount = 0;
            const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            // add glfw extensions to list
            for (uint32_t i = 0; i < glfwExtensionCount; i++) {
                wantedExtensions.push_back(glfwExtensions[i]);
            }

            // get our needed extensions
            auto installedExtensions = vk::enumerateInstanceExtensionProperties();
            auto extensions = std::vector<const char *>();
            for (auto &w : wantedExtensions) {
                for (auto &i : installedExtensions)
                {
                    if (std::string(i.extensionName).compare(w) == 0)
                    {
                        extensions.emplace_back(w);
                        break;
                    }
                }
            }

            // get validation layers
            auto installedLayers = vk::enumerateInstanceLayerProperties();
            auto layers = std::vector<const char *>();
            for (auto &w : wantedLayers) {
                for (auto &i : installedLayers)
                {
                    if (std::string(i.layerName).compare(w) == 0)
                    {
                        layers.emplace_back(w);
                        break;
                    }
                }
            }

            // app info
            vk::ApplicationInfo appinfo;
            appinfo.pApplicationName = "VKTest";
            appinfo.applicationVersion = NULL;
            appinfo.apiVersion = VK_MAKE_VERSION(1, 1, 77);

            // create instance info
            vk::InstanceCreateInfo cinstanceinfo;
            cinstanceinfo.pApplicationInfo = &appinfo;
            cinstanceinfo.enabledExtensionCount = extensions.size();
            cinstanceinfo.ppEnabledExtensionNames = extensions.data();
            cinstanceinfo.enabledLayerCount = layers.size();
            cinstanceinfo.ppEnabledLayerNames = layers.data();

            // create instance
            instance = vk::createInstance(cinstanceinfo);
            volkLoadInstance(instance);

            // get physical device for application
            return instance;
        };

        virtual Queue createDeviceQueue(vk::PhysicalDevice gpu, bool isComputePrior = false, std::string shaderPath = "./") {
            // use extensions
            auto deviceExtensions = std::vector<const char *>();
            auto gpuExtensions = gpu.enumerateDeviceExtensionProperties();
            for (auto &w : wantedDeviceExtensions) {
                for (auto &i : gpuExtensions) {
                    if (std::string(i.extensionName).compare(w) == 0) {
                        deviceExtensions.emplace_back(w);
                        break;
                    }
                }
            }

            // use layers
            auto layers = std::vector<const char *>();
            auto deviceValidationLayers = std::vector<const char *>();
            auto gpuLayers = gpu.enumerateDeviceLayerProperties();
            for (auto &w : wantedLayers) {
                for (auto &i : gpuLayers) {
                    if (std::string(i.layerName).compare(w) == 0) {
                        layers.emplace_back(w);
                        break;
                    }
                }
            }

            // get features and queue family properties
            auto gpuFeatures = gpu.getFeatures();
            auto gpuQueueProps = gpu.getQueueFamilyProperties();

            // queue family initial
            std::vector<DevQueue> queues;
            float priority = 1.0f;
            uint32_t computeFamilyIndex = -1, graphicsFamilyIndex = -1;
            auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();

            // compute/graphics queue family
            for (auto &queuefamily : gpuQueueProps) {
                computeFamilyIndex++;
                if (queuefamily.queueFlags & (vk::QueueFlagBits::eCompute)) {
                    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags()).setQueueFamilyIndex(computeFamilyIndex).setQueueCount(1).setPQueuePriorities(&priority));
                    auto devQueue = std::make_shared<DevQueueType>();
                    devQueue->familyIndex = computeFamilyIndex;
                    devQueue->queue = vk::Queue{};
                    queues.push_back(devQueue);
                    break;
                }
            }

            // graphics/presentation queue family
            for (auto &queuefamily : gpuQueueProps) {
                graphicsFamilyIndex++;
                if (queuefamily.queueFlags & (vk::QueueFlagBits::eGraphics) && gpu.getSurfaceSupportKHR(graphicsFamilyIndex, applicationWindow.surface) && graphicsFamilyIndex != computeFamilyIndex) {
                    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags()).setQueueFamilyIndex(computeFamilyIndex).setQueueCount(1).setPQueuePriorities(&priority));
                    auto devQueue = std::make_shared<DevQueueType>();
                    devQueue->familyIndex = graphicsFamilyIndex;
                    devQueue->queue = vk::Queue{};
                    queues.push_back(devQueue);
                    break;
                }
            }

            // assign presentation (may not working)
            if (int(graphicsFamilyIndex) < 0 || graphicsFamilyIndex >= gpuQueueProps.size()) {
                std::wcerr << "Device may not support presentation" << std::endl;
                graphicsFamilyIndex = computeFamilyIndex;
            }

            // make graphics queue family same as compute
            if ((graphicsFamilyIndex == computeFamilyIndex || queues.size() <= 1) && queues.size() > 0) {
                queues.push_back(queues[0]);
            }

            // pre-declare logical device
            auto devicePtr = std::make_shared<DeviceType>();

            // if have supported queue family, then use this device
            if (queueCreateInfos.size() > 0) {
                // create device
                devicePtr->physical = gpu;
                devicePtr->logical = gpu.createDevice(vk::DeviceCreateInfo().setFlags(vk::DeviceCreateFlags())
                    .setPEnabledFeatures(&gpuFeatures)
                    .setPQueueCreateInfos(queueCreateInfos.data()).setQueueCreateInfoCount(queueCreateInfos.size())
                    .setPpEnabledExtensionNames(deviceExtensions.data()).setEnabledExtensionCount(deviceExtensions.size())
                    .setPpEnabledLayerNames(deviceValidationLayers.data()).setEnabledLayerCount(deviceValidationLayers.size()));

                // load API calls for context
                volkLoadDevice(devicePtr->logical);

                // load API calls for mapping
                VolkDeviceTable vktable;
                volkLoadDeviceTable(&vktable, devicePtr->logical);

                // init dispatch loader
                devicePtr->dldid = vk::DispatchLoaderDynamic(instance, devicePtr->logical);

                // getting queues by family
                // don't get it now
                //for (int i = 0; i < queues.size(); i++) { queues[i]->queue = devicePtr->logical.getQueue(queues[i]->familyIndex, 0); }

                // mapping volk with VMA functions
                VmaVulkanFunctions vfuncs;
                vfuncs.vkAllocateMemory = vktable.vkAllocateMemory;
                vfuncs.vkBindBufferMemory = vktable.vkBindBufferMemory;
                vfuncs.vkBindImageMemory = vktable.vkBindImageMemory;
                vfuncs.vkCreateBuffer = vktable.vkCreateBuffer;
                vfuncs.vkCreateImage = vktable.vkCreateImage;
                vfuncs.vkDestroyBuffer = vktable.vkDestroyBuffer;
                vfuncs.vkDestroyImage = vktable.vkDestroyImage;
                vfuncs.vkFreeMemory = vktable.vkFreeMemory;
                vfuncs.vkGetBufferMemoryRequirements = vktable.vkGetBufferMemoryRequirements;
                vfuncs.vkGetBufferMemoryRequirements2KHR = vktable.vkGetBufferMemoryRequirements2KHR;
                vfuncs.vkGetImageMemoryRequirements = vktable.vkGetImageMemoryRequirements;
                vfuncs.vkGetImageMemoryRequirements2KHR = vktable.vkGetImageMemoryRequirements2KHR;
                vfuncs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
                vfuncs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
                vfuncs.vkMapMemory = vktable.vkMapMemory;
                vfuncs.vkUnmapMemory = vktable.vkUnmapMemory;

                // create allocator
                VmaAllocatorCreateInfo allocatorInfo = {};
                allocatorInfo.pVulkanFunctions = &vfuncs;
                allocatorInfo.physicalDevice = devicePtr->physical;
                allocatorInfo.device = devicePtr->logical;
                allocatorInfo.preferredLargeHeapBlockSize = 16384; // 16kb
                allocatorInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT || VMA_ALLOCATION_CREATE_MAPPED_BIT;
                allocatorInfo.pAllocationCallbacks = nullptr;
                allocatorInfo.pHeapSizeLimit = nullptr;
                vmaCreateAllocator(&allocatorInfo, &devicePtr->allocator);

                // init dispatch loader
                devicePtr->dldid = vk::DispatchLoaderDynamic(instance, devicePtr->logical);

                // pool sizes, and create descriptor pool
                std::vector<vk::DescriptorPoolSize> psizes = {
                    vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 64),
                    vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 16),
                    vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 16),
                    vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 128),
                    vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 16) };
                devicePtr->descriptorPool = devicePtr->logical.createDescriptorPool(vk::DescriptorPoolCreateInfo().setPPoolSizes(psizes.data()).setPoolSizeCount(psizes.size()).setMaxSets(16));
                devicePtr->pipelineCache = devicePtr->logical.createPipelineCache(vk::PipelineCacheCreateInfo());
            }

            // assign queues
            devicePtr->queues = queues;

            // return device with queue pointer
            auto deviceQueuePtr = std::make_shared<QueueType>();
            deviceQueuePtr->device = devicePtr;
            deviceQueuePtr->fence = createFence(devicePtr->logical, false);
            deviceQueuePtr->commandPool = devicePtr->logical.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer), deviceQueuePtr->familyIndex));
            deviceQueuePtr->queue = devicePtr->logical.getQueue(devicePtr->queues[isComputePrior ? 0 : 1]->familyIndex, 0); // deferred getting of queue
            deviceQueuePtr->familyIndex = devicePtr->queues[isComputePrior ? 0 : 1]->familyIndex;
            devices.push_back(deviceQueuePtr);


            { // create ray tracing instances and devices
                vt::VtInstanceConversionInfo cinfo;
                vt::VtInstance cinstance;
                vt::vtConvertInstance(instance, &cinfo, &cinstance);

                vt::VtPhysicalDevice pdevice;
                vt::vtConvertPhysicalDevice(cinstance, gpu, &pdevice);

                vt::VtArtificalDeviceExtension dbi;
                dbi.mainQueueFamily = deviceQueuePtr->familyIndex;
                dbi.shaderPath = shaderPath;
                dbi.allocator = devicePtr->allocator;
                dbi.sharedCacheSize = 1024 * 1024 * 64; // 512Mb?
                dbi.maxPrimitives = 1024 * 1024 * 4;
                vt::vtConvertDevice(pdevice, deviceQueuePtr->device->logical, &dbi, &deviceQueuePtr->device->rtDev);

                //devicePtr->allocator = deviceQueuePtr->device->rtDev->_allocator;
            }


            return std::move(deviceQueuePtr);
        }

        // create window and surface for this application (multi-window not supported)
        virtual SurfaceWindow &createWindowSurface(GLFWwindow *window, uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = window;
            applicationWindow.surfaceSize = vk::Extent2D{ WIDTH, HEIGHT };
            auto vksurface = VkSurfaceKHR();
            auto result = glfwCreateWindowSurface(instance, applicationWindow.window, nullptr, &vksurface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            applicationWindow.surface = vksurface;
            return applicationWindow;
        }

        // create window and surface for this application (multi-window not supported)
        virtual SurfaceWindow &createWindowSurface(uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
            applicationWindow.surfaceSize = vk::Extent2D{ WIDTH, HEIGHT };
            auto vksurface = VkSurfaceKHR();
            auto result = glfwCreateWindowSurface(instance, applicationWindow.window, nullptr, &vksurface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            applicationWindow.surface = vksurface;
            return applicationWindow;
        }


        // getters
        vk::SurfaceKHR surface() const {
            return applicationWindow.surface;
        }

        GLFWwindow * window() const {
            return applicationWindow.window;
        }

        const SurfaceFormat& format() const {
            return applicationWindow.surfaceFormat;
        }

        const vk::Extent2D& size() const {
            return applicationWindow.surfaceSize;
        }


        // setters
        void format(SurfaceFormat format) {
            applicationWindow.surfaceFormat = format;
        }

        void size(const vk::Extent2D& size) {
            applicationWindow.surfaceSize = size;
        }


        virtual SurfaceFormat getSurfaceFormat(vk::PhysicalDevice gpu)
        {
            auto surfaceFormats = gpu.getSurfaceFormatsKHR(applicationWindow.surface);

            const std::vector<vk::Format> preferredFormats = { vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8A8Unorm };

            vk::Format surfaceColorFormat =
                surfaceFormats.size() == 1 &&
                surfaceFormats[0].format == vk::Format::eUndefined
                ? vk::Format::eR8G8B8A8Unorm
                : surfaceFormats[0].format;

            // search preferred surface format support
            bool surfaceFormatFound = false;
            uint32_t surfaceFormatID = 0;
            for (int i = 0; i < preferredFormats.size(); i++)
            {
                if (surfaceFormatFound)
                    break;
                for (int f = 0; f < surfaceFormats.size(); f++)
                {
                    if (surfaceFormats[f].format == preferredFormats[i])
                    {
                        surfaceFormatFound = true;
                        surfaceFormatID = f;
                        break;
                    }
                }
            }

            // get supported color format
            surfaceColorFormat = surfaceFormats[surfaceFormatID].format;
            vk::ColorSpaceKHR surfaceColorSpace =
                surfaceFormats[surfaceFormatID].colorSpace;

            // get format properties?
            auto formatProperties = gpu.getFormatProperties(surfaceColorFormat);

            // only if these depth formats
            std::vector<vk::Format> depthFormats = {
                vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat,
                vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint,
                vk::Format::eD16Unorm };

            // choice supported depth format
            vk::Format surfaceDepthFormat = depthFormats[0];
            for (auto &format : depthFormats)
            {
                auto depthFormatProperties = gpu.getFormatProperties(format);
                if (depthFormatProperties.optimalTilingFeatures &
                    vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                {
                    surfaceDepthFormat = format;
                    break;
                }
            }

            // return format result
            SurfaceFormat sfd;
            sfd.colorSpace = surfaceColorSpace;
            sfd.colorFormat = surfaceColorFormat;
            sfd.depthFormat = surfaceDepthFormat;
            sfd.colorFormatProperties = formatProperties; // get properties about format
            return sfd;
        }

        virtual vk::RenderPass createRenderpass(Queue queue)
        {
            auto formats = applicationWindow.surfaceFormat;

            // attachments
            std::vector<vk::AttachmentDescription> attachmentDescriptions = {

                vk::AttachmentDescription()
                    .setFormat(formats.colorFormat)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eLoad)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),

                vk::AttachmentDescription()
                    .setFormat(formats.depthFormat)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                    .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)

            };

            // attachments references
            std::vector<vk::AttachmentReference> colorReferences = {
                vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) };
            std::vector<vk::AttachmentReference> depthReferences = {
                vk::AttachmentReference(
                    1, vk::ImageLayout::eDepthStencilAttachmentOptimal) };

            // subpasses desc
            std::vector<vk::SubpassDescription> subpasses = {
                vk::SubpassDescription()
                    .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                    .setPColorAttachments(colorReferences.data())
                    .setColorAttachmentCount(colorReferences.size())
                    .setPDepthStencilAttachment(depthReferences.data()) };

            // dependency
            std::vector<vk::SubpassDependency> dependencies = {
                vk::SubpassDependency()
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion)
                    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                     vk::PipelineStageFlagBits::eBottomOfPipe |
                                     vk::PipelineStageFlagBits::eTransfer)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)

                    .setDstSubpass(0)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite),

                vk::SubpassDependency()
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion)
                    .setSrcSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite)

                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                     vk::PipelineStageFlagBits::eTopOfPipe |
                                     vk::PipelineStageFlagBits::eTransfer)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite)

            };

            // create renderpass
            return queue->device->logical.createRenderPass(vk::RenderPassCreateInfo(
                vk::RenderPassCreateFlags(), attachmentDescriptions.size(),
                attachmentDescriptions.data(), subpasses.size(), subpasses.data(),
                dependencies.size(), dependencies.data()));
        }

        // update swapchain framebuffer
        virtual void updateSwapchainFramebuffer(Queue queue, vk::SwapchainKHR &swapchain, vk::RenderPass &renderpass, std::vector<Framebuffer> &swapchainBuffers)
        {
            // The swapchain handles allocating frame images.
            auto swapchainImages = queue->device->logical.getSwapchainImagesKHR(swapchain);
            auto gpuMemoryProps = queue->device->physical.getMemoryProperties();
            auto formats = applicationWindow.surfaceFormat;

            // create depth image
            auto imageInfo = vk::ImageCreateInfo(
                vk::ImageCreateFlags(), vk::ImageType::e2D, formats.depthFormat,
                vk::Extent3D(applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1),
                1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment |
                vk::ImageUsageFlagBits::eTransferSrc,
                vk::SharingMode::eExclusive, 1, &queue->device->queues[1]->familyIndex,
                vk::ImageLayout::eUndefined);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VmaAllocation _allocation = {};
            VkImage _image = {};
            VkImageCreateInfo imageInfoVK = imageInfo;
            vmaCreateImage(queue->device->allocator, &imageInfoVK, &allocCreateInfo, &_image, &_allocation, nullptr); // allocators planned structs
            auto depthImage = vk::Image(_image);

            // create image viewer
            auto depthImageView =
                queue->device->logical.createImageView(vk::ImageViewCreateInfo(
                    vk::ImageViewCreateFlags(), depthImage, vk::ImageViewType::e2D,
                    formats.depthFormat, vk::ComponentMapping(),
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil,
                        0, 1, 0, 1)));

            // create framebuffers
            for (int i = 0; i < swapchainImages.size(); i++)
            {
                vk::Image &image = swapchainImages[i]; // prelink images
                std::array<vk::ImageView, 2> views;    // predeclare views
                views[1] = depthImageView;

                // color view
                views[0] = queue->device->logical.createImageView(vk::ImageViewCreateInfo(
                    vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D,
                    formats.colorFormat, vk::ComponentMapping(),
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0,
                        1)));

                // create framebuffers
                swapchainBuffers[i].frameBuffer =
                    queue->device->logical.createFramebuffer(vk::FramebufferCreateInfo(
                        vk::FramebufferCreateFlags(), renderpass, views.size(),
                        views.data(), applicationWindow.surfaceSize.width,
                        applicationWindow.surfaceSize.height, 1));
            }
        }

        virtual std::vector<Framebuffer> createSwapchainFramebuffer(Queue queue, vk::SwapchainKHR swapchain, vk::RenderPass renderpass) {
            auto &formats = applicationWindow.surfaceFormat;

            // The swapchain handles allocating frame images.
            auto swapchainImages = queue->device->logical.getSwapchainImagesKHR(swapchain);
            auto gpuMemoryProps = queue->device->physical.getMemoryProperties();

            // create depth image
            auto imageInfo = vk::ImageCreateInfo(
                vk::ImageCreateFlags(), vk::ImageType::e2D, formats.depthFormat,
                vk::Extent3D(applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1),
                1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment |
                vk::ImageUsageFlagBits::eTransferSrc,
                vk::SharingMode::eExclusive, 1, &queue->device->queues[1]->familyIndex,
                vk::ImageLayout::eUndefined);
            VkImageCreateInfo imageInfoVK = imageInfo;

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VmaAllocation _allocation = {};
            VkImage _image = {};
            vmaCreateImage(queue->device->allocator, &imageInfoVK, &allocCreateInfo, &_image, &_allocation, nullptr); // allocators planned structs
            auto depthImage = vk::Image(_image);

            // create image viewer
            auto depthImageView =
                queue->device->logical.createImageView(vk::ImageViewCreateInfo(
                    vk::ImageViewCreateFlags(), depthImage, vk::ImageViewType::e2D,
                    formats.depthFormat, vk::ComponentMapping(),
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil,
                        0, 1, 0, 1)));

            // framebuffers vector
            std::vector<Framebuffer> swapchainBuffers;
            swapchainBuffers.resize(swapchainImages.size());

            // create framebuffers
            for (int i = 0; i < swapchainImages.size(); i++)
            {
                vk::Image &image = swapchainImages[i]; // prelink images
                std::array<vk::ImageView, 2> views;    // predeclare views

                // depth view
                views[1] = depthImageView;

                // color view
                views[0] = queue->device->logical.createImageView(vk::ImageViewCreateInfo(
                    vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D,
                    formats.colorFormat, vk::ComponentMapping(),
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0,
                        1)));

                // create framebuffers
                swapchainBuffers[i].frameBuffer =
                    queue->device->logical.createFramebuffer(vk::FramebufferCreateInfo(
                        vk::FramebufferCreateFlags(), renderpass, views.size(),
                        views.data(), applicationWindow.surfaceSize.width,
                        applicationWindow.surfaceSize.height, 1));

                // create semaphore
                swapchainBuffers[i].semaphore =
                    queue->device->logical.createSemaphore(vk::SemaphoreCreateInfo());
                swapchainBuffers[i].waitFence = queue->device->logical.createFence(
                    vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
            }

            return swapchainBuffers;
        }

        // create swapchain template
        virtual vk::SwapchainKHR createSwapchain(Queue queue)
        {
            vk::SurfaceKHR surface = applicationWindow.surface;
            SurfaceFormat &formats = applicationWindow.surfaceFormat;

            auto surfaceCapabilities = queue->device->physical.getSurfaceCapabilitiesKHR(surface);
            auto surfacePresentModes = queue->device->physical.getSurfacePresentModesKHR(surface);

            // check the surface width/height.
            if (!(surfaceCapabilities.currentExtent.width == -1 ||
                surfaceCapabilities.currentExtent.height == -1))
            {
                applicationWindow.surfaceSize = surfaceCapabilities.currentExtent;
            }

            // get supported present mode, but prefer mailBox
            auto presentMode = vk::PresentModeKHR::eImmediate;
            std::vector<vk::PresentModeKHR> priorityModes = { vk::PresentModeKHR::eImmediate, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifoRelaxed, vk::PresentModeKHR::eFifo };

            bool found = false;
            for (auto &pm : priorityModes) {
                if (found) break;
                for (auto &sfm : surfacePresentModes) {
                    if (pm == sfm) { presentMode = pm; found = true; break; }
                }
            }

            // swapchain info
            auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR();
            swapchainCreateInfo.surface = surface;
            swapchainCreateInfo.minImageCount = std::min(surfaceCapabilities.maxImageCount, 3u);
            swapchainCreateInfo.imageFormat = formats.colorFormat;
            swapchainCreateInfo.imageColorSpace = formats.colorSpace;
            swapchainCreateInfo.imageExtent = applicationWindow.surfaceSize;
            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
            swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
            swapchainCreateInfo.queueFamilyIndexCount = 1;
            swapchainCreateInfo.pQueueFamilyIndices = &queue->device->queues[1]->familyIndex;
            swapchainCreateInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
            swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            swapchainCreateInfo.presentMode = presentMode;
            swapchainCreateInfo.clipped = true;

            // create swapchain
            return queue->device->logical.createSwapchainKHR(swapchainCreateInfo);
        }
    };

}; // namespace NSM