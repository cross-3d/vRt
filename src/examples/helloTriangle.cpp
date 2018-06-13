#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#define __INT32_TYPE__
#define USE_SWAPCHAIN_EXTENSIONS

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_LEFT_HANDED

#define DEFAULT_FENCE_TIMEOUT 100000000000
#include "windows.h"

#include <chrono>
#include <exception>
#include <algorithm>
#include <fstream>

#include "vulkan/vulkan.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main(const int argc, const char *argv[])
{

#pragma region StartInstance

    auto installedExtensions = vk::enumerateInstanceExtensionProperties();

    std::vector<const char*> wantedExtensions =
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    auto extensions = std::vector<const char*>();

    for (auto &w : wantedExtensions) {
        for (auto &i : installedExtensions) {
            if (std::string(i.extensionName).compare(w) == 0) {
                extensions.emplace_back(w);
                break;
            }
        }
    }

    auto installedLayers = vk::enumerateInstanceLayerProperties();

    std::vector<const char*> wantedLayers =
    {
        "VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_RENDERDOC_Capture"
    };

    auto layers = std::vector<const char*>();

    for (auto &w : wantedLayers) {
        for (auto &i : installedLayers) {
            if (std::string(i.layerName).compare(w) == 0) {
                layers.emplace_back(w);
                break;
            }
        }
    }

    auto appInfo = vk::ApplicationInfo(
        "MyApp",
        VK_MAKE_VERSION(1, 0, 0),
        "MyAppEngine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0
    );

    auto instance = vk::createInstance(
        vk::InstanceCreateInfo(
            vk::InstanceCreateFlags(),
            &appInfo,
            layers.size(),
            layers.data(),
            extensions.size(),
            extensions.data()
        )
    );

#pragma endregion

#pragma region PhysicalDevices

    // Initialize Devices
    auto physicalDevices = instance.enumeratePhysicalDevices();
    auto gpu = physicalDevices[0];
    auto gpuProps = gpu.getProperties();
    auto gpuMemoryProps = gpu.getMemoryProperties();

#pragma endregion

#pragma region LogicalDevice
    auto gpuExtensions = gpu.enumerateDeviceExtensionProperties();

    // Init Device Extension/Validation layers
    std::vector<const char*> wantedDeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DEBUG_MARKER_EXTENSION_NAME
    };

    auto deviceExtensions = std::vector<const char*>();

    for (auto &w : wantedDeviceExtensions) {
        for (auto &i : gpuExtensions) {
            if (std::string(i.extensionName).compare(w) == 0) {
                deviceExtensions.emplace_back(w);
                break;
            }
        }
    }

    auto gpuLayers = gpu.enumerateDeviceLayerProperties();

    std::vector<const char*> wantedDeviceValidationLayers =
    {
        "VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_RENDERDOC_Capture"
    };

    auto deviceValidationLayers = std::vector<const char*>();

    for (auto &w : wantedLayers) {
        for (auto &i : installedLayers) {
            if (std::string(i.layerName).compare(w) == 0) {
                layers.emplace_back(w);
                break;
            }
        }
    }



    auto gpuFeatures = gpu.getFeatures();
    auto gpuQueueProps = gpu.getQueueFamilyProperties();

    float priority = 0.0;
    uint32_t graphicsFamilyIndex = 0;
    auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();

    for (auto& queuefamily : gpuQueueProps)
    {
        if (queuefamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            // Create a single graphics queue.
            queueCreateInfos.push_back(
                vk::DeviceQueueCreateInfo(
                    vk::DeviceQueueCreateFlags(),
                    graphicsFamilyIndex,
                    1,
                    &priority
                )
            );
            break;
        }

        graphicsFamilyIndex++;

    }

    auto device = gpu.createDevice(
        vk::DeviceCreateInfo(
            vk::DeviceCreateFlags(),
            queueCreateInfos.size(),
            queueCreateInfos.data(),
            deviceValidationLayers.size(),
            deviceValidationLayers.data(),
            deviceExtensions.size(),
            deviceExtensions.data(),
            &gpuFeatures
        )
    );

#pragma endregion

#pragma region Queue
    auto graphicsQueue = device.getQueue(graphicsFamilyIndex, 0);
#pragma endregion

#pragma region Window
    std::string title = "MyVulkanApp";
    std::string name = "MyVulkanApp";
    uint32_t width = 1280;
    uint32_t height = 720;
    auto hInstance = GetModuleHandle(0);

    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = [](HWND h, UINT m, WPARAM w, LPARAM l)->LRESULT
    {
        if (m == WM_CLOSE)
            PostQuitMessage(0);
        else
            return DefWindowProc(h, m, w, l);
        return 0;
    };
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = name.c_str();
    wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

    if (!RegisterClassEx(&wndClass)) {
        fflush(stdout);
        exit(1);
    }

    DWORD dwExStyle;
    DWORD dwStyle;

    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    RECT windowRect;
    windowRect.left = 0L;
    windowRect.top = 0L;
    windowRect.right = (long)width;
    windowRect.bottom = (long)height;

    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

    auto window = CreateWindowEx(0,
        name.c_str(),
        title.c_str(),
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0,
        0,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    // Center on screen
    uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
    uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
    SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    if (!window) {
        printf("Could not create window!\n");
        fflush(stdout);
        exit(1);
    }

    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);
    SetFocus(window);
#pragma endregion

#pragma region Surface

    // Screen Size
    auto surfaceSize = vk::Extent2D(width, height);
    auto renderArea = vk::Rect2D(vk::Offset2D(), surfaceSize);
    auto viewport = vk::Viewport(0.0f, 0.0f, width, height, 0, 1.0f);

    std::vector<vk::Viewport> viewports =
    {
        viewport
    };

    std::vector<vk::Rect2D> scissors =
    {
        renderArea
    };

    auto surfaceInfo = vk::Win32SurfaceCreateInfoKHR(vk::Win32SurfaceCreateFlagsKHR(), hInstance, window);
    auto vkSurfaceInfo = surfaceInfo.operator const VkWin32SurfaceCreateInfoKHR&();

    auto vksurface = VkSurfaceKHR();
    auto createwin32surface = vkCreateWin32SurfaceKHR(instance, &vkSurfaceInfo, NULL, &vksurface);
    assert(createwin32surface == VK_SUCCESS);

    // Get surface information
    auto surface = vk::SurfaceKHR(vksurface);

#pragma endregion

#pragma region ColorFormats
    // Check to see if we can display rgb colors.
    auto surfaceFormats = gpu.getSurfaceFormatsKHR(surface);

    vk::Format surfaceColorFormat;
    vk::ColorSpaceKHR surfaceColorSpace;

    if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
        surfaceColorFormat = vk::Format::eB8G8R8A8Unorm;
    else
        surfaceColorFormat = surfaceFormats[0].format;

    surfaceColorSpace = surfaceFormats[0].colorSpace;


    auto formatProperties = gpu.getFormatProperties(vk::Format::eR8G8B8A8Unorm);

    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<vk::Format> depthFormats = {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD32Sfloat,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD16Unorm
    };

    vk::Format surfaceDepthFormat;

    for (auto& format : depthFormats)
    {
        auto depthFormatProperties = gpu.getFormatProperties(format);
        // Format must support depth stencil attachment for optimal tiling
        if (depthFormatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            surfaceDepthFormat = format;
            break;
        }
    }
#pragma endregion

#pragma region RenderPass
    std::vector<vk::AttachmentDescription> attachmentDescriptions =
    {
        vk::AttachmentDescription(
            vk::AttachmentDescriptionFlags(),
            surfaceColorFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
        ),
        vk::AttachmentDescription(
            vk::AttachmentDescriptionFlags(),
            surfaceDepthFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
        )
    };

    std::vector<vk::AttachmentReference> colorReferences =
    {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal)
    };

    std::vector<vk::AttachmentReference> depthReferences = {
        vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
    };

    std::vector<vk::SubpassDescription> subpasses =
    {
        vk::SubpassDescription(
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            0,
            nullptr,
            colorReferences.size(),
            colorReferences.data(),
            nullptr,
            depthReferences.data(),
            0,
            nullptr
        )
    };

    std::vector<vk::SubpassDependency> dependencies =
    {
        vk::SubpassDependency(
            ~0U,
            0,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        ),
        vk::SubpassDependency(
            0,
            ~0U,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::DependencyFlagBits::eByRegion
        )
    };

    auto renderpass = device.createRenderPass(
        vk::RenderPassCreateInfo(
            vk::RenderPassCreateFlags(),
            attachmentDescriptions.size(),
            attachmentDescriptions.data(),
            subpasses.size(),
            subpasses.data(),
            dependencies.size(),
            dependencies.data()
        )
    );

#pragma endregion

#pragma region Swapchain

    auto surfaceCapabilities = gpu.getSurfaceCapabilitiesKHR(surface);
    auto surfacePresentModes = gpu.getSurfacePresentModesKHR(surface);

    // check the surface width/height.
    if (!(surfaceCapabilities.currentExtent.width == -1 || surfaceCapabilities.currentExtent.height == -1)) {
        surfaceSize = surfaceCapabilities.currentExtent;
    }

    auto presentMode = vk::PresentModeKHR::eImmediate;

    for (auto& pm : surfacePresentModes) {
        if (pm == vk::PresentModeKHR::eMailbox) {
            presentMode = vk::PresentModeKHR::eMailbox;
            break;
        }
    }

    assert(surfaceCapabilities.maxImageCount >= 3);


    std::vector<uint32_t> queueFamilyIndices;
    queueFamilyIndices.push_back(graphicsFamilyIndex);

    auto swapchain = device.createSwapchainKHR(
        vk::SwapchainCreateInfoKHR(
            vk::SwapchainCreateFlagsKHR(),
            surface,
            surfaceCapabilities.maxImageCount,
            surfaceColorFormat,
            surfaceColorSpace,
            surfaceSize,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive,
            queueFamilyIndices.size(),
            queueFamilyIndices.data(),
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            presentMode,
            VK_TRUE,
            vk::SwapchainKHR()
        )
    );

#pragma endregion

#pragma region FrameBuffers
    // The swapchain handles allocating frame images.
    auto swapchainImages = device.getSwapchainImagesKHR(swapchain);

    // Create Depth Image Data
    auto depthImage = device.createImage(
        vk::ImageCreateInfo(
            vk::ImageCreateFlags(),
            vk::ImageType::e2D,
            surfaceDepthFormat,
            vk::Extent3D(surfaceSize.width, surfaceSize.height, 1),
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive,
            queueFamilyIndices.size(),
            queueFamilyIndices.data(),
            vk::ImageLayout::eUndefined
        )
    );

    auto depthMemoryReq = device.getImageMemoryRequirements(depthImage);

    // Search through GPU memory properies to see if this can be device local.

    auto getMemoryTypeIndex = [&](uint32_t typeBits, vk::MemoryPropertyFlags properties)
    {
        for (uint32_t i = 0; i < gpuMemoryProps.memoryTypeCount; i++)
        {
            if ((typeBits & 1) == 1)
            {
                if ((gpuMemoryProps.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            typeBits >>= 1;
        }
    };

    auto depthMemory = device.allocateMemory(
        vk::MemoryAllocateInfo(
            depthMemoryReq.size,
            getMemoryTypeIndex(depthMemoryReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
        )
    );


    device.bindImageMemory(
        depthImage,
        depthMemory,
        0
    );

    auto depthImageView = device.createImageView(
        vk::ImageViewCreateInfo(
            vk::ImageViewCreateFlags(),
            depthImage,
            vk::ImageViewType::e2D,
            surfaceDepthFormat,
            vk::ComponentMapping(),
            vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
                0,
                1,
                0,
                1
            )
        )
    );

    struct SwapChainBuffer {
        vk::Image image;
        std::array<vk::ImageView, 2> views;
        vk::Framebuffer frameBuffer;
    };

    std::vector<SwapChainBuffer> swapchainBuffers;
    swapchainBuffers.resize(swapchainImages.size());

    for (int i = 0; i < swapchainImages.size(); i++)
    {
        swapchainBuffers[i].image = swapchainImages[i];

        // Color
        swapchainBuffers[i].views[0] =
            device.createImageView(
                vk::ImageViewCreateInfo(
                    vk::ImageViewCreateFlags(),
                    swapchainImages[i],
                    vk::ImageViewType::e2D,
                    surfaceColorFormat,
                    vk::ComponentMapping(),
                    vk::ImageSubresourceRange(
                        vk::ImageAspectFlagBits::eColor,
                        0,
                        1,
                        0,
                        1
                    )
                )
            );

        // Depth
        swapchainBuffers[i].views[1] = depthImageView;

        swapchainBuffers[i].frameBuffer = device.createFramebuffer(
            vk::FramebufferCreateInfo(
                vk::FramebufferCreateFlags(),
                renderpass,
                swapchainBuffers[i].views.size(),
                swapchainBuffers[i].views.data(),
                surfaceSize.width,
                surfaceSize.height,
                1
            )
        );
    }
#pragma endregion

#pragma region Synchronization

    // Semaphore used to ensures that image presentation is complete before starting to submit again
    auto presentCompleteSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());

    // Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
    auto renderCompleteSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());

    // Fence for command buffer completion
    std::vector<vk::Fence> waitFences;
    waitFences.resize(swapchainBuffers.size());
    for (int i = 0; i < waitFences.size(); i++)
    {
        waitFences[i] = device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

#pragma endregion

#pragma region CommandPool

    auto commandPool = device.createCommandPool(
        vk::CommandPoolCreateInfo(
            vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
            graphicsFamilyIndex
        )
    );

    // Allocate one buffer for each frame in the Swapchain.
    auto commandBuffers = device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo(
            commandPool,
            vk::CommandBufferLevel::ePrimary,
            swapchainBuffers.size()
        )
    );
#pragma endregion

#pragma region DescriptorPool
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
    {
        vk::DescriptorPoolSize(
            vk::DescriptorType::eUniformBuffer,
            1
        )
    };

    auto descriptorPool = device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo(
            vk::DescriptorPoolCreateFlags(),
            1,
            descriptorPoolSizes.size(),
            descriptorPoolSizes.data()
        )
    );
#pragma endregion

#pragma region DescriptorSetLayout
    // Binding 0: Uniform buffer (Vertex shader)
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings =
    {
        vk::DescriptorSetLayoutBinding(
            0,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eVertex,
            nullptr
        )
    };

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
        device.createDescriptorSetLayout(
            vk::DescriptorSetLayoutCreateInfo(
                vk::DescriptorSetLayoutCreateFlags(),
                descriptorSetLayoutBindings.size(),
                descriptorSetLayoutBindings.data()
            )
        )
    };

    auto descriptorSets = device.allocateDescriptorSets(
        vk::DescriptorSetAllocateInfo(
            descriptorPool,
            descriptorSetLayouts.size(),
            descriptorSetLayouts.data()
        )
    );
#pragma endregion

#pragma region VertexBuffer

    struct Vertex
    {
        float position[3];
        float color[3];
    };

    // Vertex buffer and attributes
    struct {
        vk::DeviceMemory memory;															// Handle to the device memory for this buffer
        vk::Buffer buffer;																// Handle to the Vulkan buffer object that the memory is bound to
        vk::PipelineVertexInputStateCreateInfo inputState;
        vk::VertexInputBindingDescription inputBinding;
        std::vector<vk::VertexInputAttributeDescription> inputAttributes;
    } vertices;

    // Index buffer
    struct
    {
        vk::DeviceMemory memory;
        vk::Buffer buffer;
        uint32_t count;
    } indices;

    // Uniform block object
    struct {
        vk::DeviceMemory memory;
        vk::Buffer buffer;
        vk::DescriptorBufferInfo descriptor;
    }  uniformDataVS;

    // For simplicity we use the same uniform block layout as in the shader:
    //
    //	layout(set = 0, binding = 0) uniform UBO
    //	{
    //		mat4 projectionMatrix;
    //		mat4 modelMatrix;
    //		mat4 viewMatrix;
    //	} ubo;
    //
    // This way we can just memcopy the ubo data to the ubo
    // Note: You should use data types that align with the GPU in order to avoid manual padding (vec4, mat4)
    struct {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    } uboVS;

    // Setup vertices data
    std::vector<Vertex> vertexBuffer =
    {
        { { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
    { { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
    { { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
    };

    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

    // Setup indices data
    std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

    void *data;
    // Static data like vertex and index buffer should be stored on the device memory 
    // for optimal (and fastest) access by the GPU
    //
    // To achieve this we use so-called "staging buffers" :
    // - Create a buffer that's visible to the host (and can be mapped)
    // - Copy the data to this buffer
    // - Create another buffer that's local on the device (VRAM) with the same size
    // - Copy the data from the host to the device using a command buffer
    // - Delete the host visible (staging) buffer
    // - Use the device local buffers for rendering

    struct StagingBuffer {
        vk::DeviceMemory memory;
        vk::Buffer buffer;
    };

    struct {
        StagingBuffer vertices;
        StagingBuffer indices;
    } stagingBuffers;

    // Vertex buffer
    stagingBuffers.vertices.buffer = device.createBuffer(
        vk::BufferCreateInfo(
            vk::BufferCreateFlags(),
            vertexBufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive,
            queueFamilyIndices.size(),
            queueFamilyIndices.data()
        )
    );

    auto memReqs = device.getBufferMemoryRequirements(stagingBuffers.vertices.buffer);

    // Request a host visible memory type that can be used to copy our data do
    // Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
    stagingBuffers.vertices.memory = device.allocateMemory(
        vk::MemoryAllocateInfo(
            memReqs.size,
            getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
        )
    );

    // Map and copy
    data = device.mapMemory(stagingBuffers.vertices.memory, 0, memReqs.size, vk::MemoryMapFlags());
    memcpy(data, vertexBuffer.data(), vertexBufferSize);
    device.unmapMemory(stagingBuffers.vertices.memory);
    device.bindBufferMemory(stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0);

    // Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
    vertices.buffer = device.createBuffer(
        vk::BufferCreateInfo(
            vk::BufferCreateFlags(),
            vertexBufferSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive,
            queueFamilyIndices.size(),
            queueFamilyIndices.data()
        )
    );

    memReqs = device.getBufferMemoryRequirements(vertices.buffer);

    vertices.memory = device.allocateMemory(
        vk::MemoryAllocateInfo(
            memReqs.size,
            getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
        )
    );

    device.bindBufferMemory(vertices.buffer, vertices.memory, 0);

    // Index buffer
    // Copy index data to a buffer visible to the host (staging buffer)
    stagingBuffers.indices.buffer = device.createBuffer(
        vk::BufferCreateInfo(
            vk::BufferCreateFlags(),
            indexBufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive,
            queueFamilyIndices.size(),
            queueFamilyIndices.data()
        )
    );
    memReqs = device.getBufferMemoryRequirements(stagingBuffers.indices.buffer);
    stagingBuffers.indices.memory = device.allocateMemory(
        vk::MemoryAllocateInfo(
            memReqs.size,
            getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
        )
    );

    data = device.mapMemory(stagingBuffers.indices.memory, 0, indexBufferSize, vk::MemoryMapFlags());
    memcpy(data, indexBuffer.data(), indexBufferSize);
    device.unmapMemory(stagingBuffers.indices.memory);
    device.bindBufferMemory(stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0);

    // Create destination buffer with device only visibility
    indices.buffer = device.createBuffer(
        vk::BufferCreateInfo(
            vk::BufferCreateFlags(),
            indexBufferSize,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
            vk::SharingMode::eExclusive,
            0,
            nullptr
        )
    );

    memReqs = device.getBufferMemoryRequirements(indices.buffer);
    indices.memory = device.allocateMemory(
        vk::MemoryAllocateInfo(
            memReqs.size,
            getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal
            )
        )
    );

    device.bindBufferMemory(indices.buffer, indices.memory, 0);

    auto getCommandBuffer = [&](bool begin)
    {
        vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers(
            vk::CommandBufferAllocateInfo(
                commandPool,
                vk::CommandBufferLevel::ePrimary,
                1)
        )[0];

        // If requested, also start the new command buffer
        if (begin)
        {
            cmdBuffer.begin(
                vk::CommandBufferBeginInfo()
            );
        }

        return cmdBuffer;
    };

    // Buffer copies have to be submitted to a queue, so we need a command buffer for them
    // Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
    vk::CommandBuffer copyCmd = getCommandBuffer(true);

    // Put buffer region copies into command buffer
    std::vector<vk::BufferCopy> copyRegions =
    {
        vk::BufferCopy(0, 0, vertexBufferSize)
    };

    // Vertex buffer
    copyCmd.copyBuffer(stagingBuffers.vertices.buffer, vertices.buffer, copyRegions);

    // Index buffer
    copyRegions =
    {
        vk::BufferCopy(0, 0,  indexBufferSize)
    };

    copyCmd.copyBuffer(stagingBuffers.indices.buffer, indices.buffer, copyRegions);

    // Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
    auto flushCommandBuffer = [&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.end();

        std::vector<vk::SubmitInfo> submitInfos = {
            vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr)
        };

        // Create fence to ensure that the command buffer has finished executing
        vk::Fence fence = device.createFence(vk::FenceCreateInfo());

        // Submit to the queue
        graphicsQueue.submit(submitInfos, fence);
        // Wait for the fence to signal that command buffer has finished executing
        device.waitForFences(1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
        device.destroyFence(fence);
        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
    };

    flushCommandBuffer(copyCmd);

    // Destroy staging buffers
    // Note: Staging buffer must not be deleted before the copies have been submitted and executed
    device.destroyBuffer(stagingBuffers.vertices.buffer);
    device.freeMemory(stagingBuffers.vertices.memory);
    device.destroyBuffer(stagingBuffers.indices.buffer);
    device.freeMemory(stagingBuffers.indices.memory);


    // Vertex input binding
    vertices.inputBinding.binding = 0;
    vertices.inputBinding.stride = sizeof(Vertex);
    vertices.inputBinding.inputRate = vk::VertexInputRate::eVertex;

    // Inpute attribute binding describe shader attribute locations and memory layouts
    // These match the following shader layout (see triangle.vert):
    //	layout (location = 0) in vec3 inPos;
    //	layout (location = 1) in vec3 inColor;
    vertices.inputAttributes.resize(2);
    // Attribute location 0: Position
    vertices.inputAttributes[0].binding = 0;
    vertices.inputAttributes[0].location = 0;
    vertices.inputAttributes[0].format = vk::Format::eR32G32B32Sfloat;
    vertices.inputAttributes[0].offset = offsetof(Vertex, position);
    // Attribute location 1: Color
    vertices.inputAttributes[1].binding = 0;
    vertices.inputAttributes[1].location = 1;
    vertices.inputAttributes[1].format = vk::Format::eR32G32B32Sfloat;
    vertices.inputAttributes[1].offset = offsetof(Vertex, color);

    // Assign to the vertex input state used for pipeline creation
    vertices.inputState.flags = vk::PipelineVertexInputStateCreateFlags();
    vertices.inputState.vertexBindingDescriptionCount = 1;
    vertices.inputState.pVertexBindingDescriptions = &vertices.inputBinding;
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.inputAttributes.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.inputAttributes.data();
#pragma endregion

#pragma region UniformBuffers
    // Prepare and initialize a uniform buffer block containing shader uniforms
    // Single uniforms like in OpenGL are no longer present in Vulkan. All Shader uniforms are passed via uniform buffer blocks

    // Vertex shader uniform buffer block
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = 0;
    allocInfo.memoryTypeIndex = 0;

    // Create a new buffer
    uniformDataVS.buffer = device.createBuffer(
        vk::BufferCreateInfo(
            vk::BufferCreateFlags(),
            sizeof(uboVS),
            vk::BufferUsageFlagBits::eUniformBuffer
        )
    );
    // Get memory requirements including size, alignment and memory type 
    memReqs = device.getBufferMemoryRequirements(uniformDataVS.buffer);
    allocInfo.allocationSize = memReqs.size;
    // Get the memory type index that supports host visibile memory access
    // Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
    // We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
    // Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    // Allocate memory for the uniform buffer
    uniformDataVS.memory = device.allocateMemory(allocInfo);
    // Bind memory to buffer
    device.bindBufferMemory(uniformDataVS.buffer, uniformDataVS.memory, 0);

    // Store information in the uniform's descriptor that is used by the descriptor set
    uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
    uniformDataVS.descriptor.offset = 0;
    uniformDataVS.descriptor.range = sizeof(uboVS);
#pragma endregion

#pragma region UpdateUniforms

    float zoom = -2.5f;
    auto rotation = glm::vec3();

    // Update matrices
    uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);

    uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));

    uboVS.modelMatrix = glm::mat4();
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Map uniform buffer and update it
    void *pData;
    pData = device.mapMemory(uniformDataVS.memory, 0, sizeof(uboVS));
    memcpy(pData, &uboVS, sizeof(uboVS));
    device.unmapMemory(uniformDataVS.memory);

#pragma endregion

#pragma region DescriptorSetUpdate
    std::vector<vk::WriteDescriptorSet> descriptorWrites =
    {
        vk::WriteDescriptorSet(
            descriptorSets[0],
            0,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &uniformDataVS.descriptor,
            nullptr
        )
    };

    device.updateDescriptorSets(descriptorWrites, nullptr);
#pragma endregion

#pragma region LoadShaders
    static auto readFile = [&](const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        bool exists = (bool)file;

        if (!exists || !file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    };

    auto vertShaderCode = readFile("C:/Users/Joshua Figuereo/Documents/Visual Studio 2017/Projects/raw-vulkan-examples/assets/triangle.vert.spv");
    auto fragShaderCode = readFile("C:/Users/Joshua Figuereo/Documents/Visual Studio 2017/Projects/raw-vulkan-examples/assets/triangle.frag.spv");
#pragma endregion

#pragma region Pipeline

    auto pipelineLayout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo(
            vk::PipelineLayoutCreateFlags(),
            descriptorSetLayouts.size(),
            descriptorSetLayouts.data(),
            0,
            nullptr
        )
    );

    auto vertModule = device.createShaderModule(
        vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(),
            vertShaderCode.size(),
            (uint32_t*)vertShaderCode.data()
        )
    );

    auto fragModule = device.createShaderModule(
        vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(),
            fragShaderCode.size(),
            (uint32_t*)fragShaderCode.data()
        )
    );

    auto pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

    std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStages = {
        vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(),
            vk::ShaderStageFlagBits::eVertex,
            vertModule,
            "main",
            nullptr
        ),
        vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(),
            vk::ShaderStageFlagBits::eFragment,
            fragModule,
            "main",
            nullptr
        )
    };

    auto pvi = vertices.inputState;

    auto pia = vk::PipelineInputAssemblyStateCreateInfo(
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList
    );

    auto pt = vk::PipelineTessellationStateCreateInfo(
    );

    auto pv = vk::PipelineViewportStateCreateInfo(
        vk::PipelineViewportStateCreateFlagBits(),
        viewports.size(),
        viewports.data(),
        scissors.size(),
        scissors.data()
    );

    auto pr = vk::PipelineRasterizationStateCreateInfo(
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE,
        VK_FALSE,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise,
        VK_FALSE,
        0,
        0,
        0,
        1.0f
    );

    auto pm = vk::PipelineMultisampleStateCreateInfo(
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1
    );

    // Dept and Stencil state for primative compare/test operations

    auto pds = vk::PipelineDepthStencilStateCreateInfo(
        vk::PipelineDepthStencilStateCreateFlags(),
        VK_TRUE,
        VK_TRUE,
        vk::CompareOp::eLessOrEqual,
        VK_FALSE,
        VK_FALSE,
        vk::StencilOpState(),
        vk::StencilOpState(),
        0,
        0
    );

    // Blend State - How two primatives should draw on top of each other.
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments =
    {
        vk::PipelineColorBlendAttachmentState(
            VK_FALSE,
            vk::BlendFactor::eZero,
            vk::BlendFactor::eOne,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eZero,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::ColorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
        )
    };

    auto pbs = vk::PipelineColorBlendStateCreateInfo(
        vk::PipelineColorBlendStateCreateFlags(),
        0,
        vk::LogicOp::eClear,
        colorBlendAttachments.size(),
        colorBlendAttachments.data()
    );

    std::vector<vk::DynamicState> dynamicStates =
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    auto pdy = vk::PipelineDynamicStateCreateInfo(
        vk::PipelineDynamicStateCreateFlags(),
        dynamicStates.size(),
        dynamicStates.data()
    );

    auto graphicsPipeline = device.createGraphicsPipeline(pipelineCache,
        vk::GraphicsPipelineCreateInfo(
            vk::PipelineCreateFlags(vk::PipelineCreateFlagBits::eDerivative),
            pipelineShaderStages.size(),
            pipelineShaderStages.data(),
            &pvi,
            &pia,
            nullptr,
            &pv,
            &pr,
            &pm,
            &pds,
            &pbs,
            &pdy,
            pipelineLayout,
            renderpass,
            0
        )
    );

#pragma endregion

#pragma region Commands

    std::vector<vk::ClearValue> clearValues =
    {
        vk::ClearColorValue(
            std::array<float,4>{0.2f, 0.2f, 0.2f, 1.0f}
    ),
        vk::ClearDepthStencilValue(1.0f, 0)
    };

    // From here we can do common GL commands
    // Lets add commands to each command buffer.
    for (int32_t i = 0; i < commandBuffers.size(); ++i)
    {
        commandBuffers[i].begin(vk::CommandBufferBeginInfo());
        commandBuffers[i].beginRenderPass(
            vk::RenderPassBeginInfo(
                renderpass,
                swapchainBuffers[i].frameBuffer,
                renderArea,
                clearValues.size(),
                clearValues.data()
            ),
            vk::SubpassContents::eInline
        );

        commandBuffers[i].setViewport(0, viewports);

        commandBuffers[i].setScissor(0, scissors);

        // Bind Descriptor Sets, these are attribute/uniform "descriptions"
        commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        commandBuffers[i].bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipelineLayout,
            0,
            descriptorSets,
            nullptr
        );
        std::array<vk::DeviceSize, 1> offsets = { 0 };
        commandBuffers[i].bindVertexBuffers(0, 1, &vertices.buffer, offsets.data());
        commandBuffers[i].bindIndexBuffer(indices.buffer, 0, vk::IndexType::eUint32);
        commandBuffers[i].drawIndexed(indices.count, 1, 0, 0, 1);
        commandBuffers[i].endRenderPass();
        commandBuffers[i].end();
    }
#pragma endregion

#pragma region SubmitCommandBuffers
    uint32_t currentBuffer = 0;
    uint32_t imageIndex = 0;
#pragma endregion

#pragma region RenderLoop
    MSG msg;
    uint64_t frameCounter = 0;
    double frameTimer = 0.0;
    double fpsTimer = 0.0;
    double lastFPS = 0.0;

    while (TRUE)
    {
        auto tStart = std::chrono::high_resolution_clock::now();

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
        {
            break;
        }

        device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), presentCompleteSemaphore, nullptr, &currentBuffer);
        device.waitForFences(1, &waitFences[currentBuffer], VK_TRUE, UINT64_MAX);
        device.resetFences(1, &waitFences[currentBuffer]);

        // Create kernels to submit to the queue on a given render pass.
        vk::PipelineStageFlags kernelPipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        auto kernel = vk::SubmitInfo(
            1,
            &presentCompleteSemaphore,
            &kernelPipelineStageFlags,
            1,
            &commandBuffers[currentBuffer],
            1,
            &renderCompleteSemaphore
        );

        graphicsQueue.submit(1, &kernel, waitFences[currentBuffer]);
        graphicsQueue.presentKHR(
            vk::PresentInfoKHR(
                1,
                &renderCompleteSemaphore,
                1,
                &swapchain,
                &currentBuffer,
                nullptr
            )
        );

        frameCounter++;
        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        frameTimer = tDiff / 1000.0;

        fpsTimer += tDiff;
        if (fpsTimer > 1000.0)
        {
            std::string windowTitle = title + " - " + std::to_string(frameCounter) + " fps";
            SetWindowText(window, windowTitle.c_str());

            lastFPS = roundf(1.0 / frameTimer);
            fpsTimer = 0.0;
            frameCounter = 0;
        }
    }
#pragma endregion

    device.waitIdle();

    return 0;
}