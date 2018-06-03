#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vt;


    inline VtResult makePhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, VtPhysicalDevice& _vtPhysicalDevice){
        _vtPhysicalDevice._vtPhysicalDevice = std::make_shared<PhysicalDevice>();
        _vtPhysicalDevice._vtPhysicalDevice->_physicalDevice = physical; // assign a Vulkan physical device
        return VT_SUCCESS;
    };

    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, const VtDeviceCreateInfo& vdvi, VtDevice& _vtDevice){
        _vtDevice._vtDevice = std::make_shared<Device>();

        auto& vtDevice = _vtDevice._vtDevice; vtDevice->_physicalDevice = physicalDevice; // reference for aliasing
        

        VtResult result = VT_ERROR_INITIALIZATION_FAILED;

        VkDeviceCreateInfo dvi;
        dvi.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dvi.queueCreateInfoCount = vdvi.queueCreateInfoCount;
        dvi.pQueueCreateInfos = vdvi.pQueueCreateInfos;
        dvi.enabledLayerCount = vdvi.enabledLayerCount;
        dvi.ppEnabledLayerNames = vdvi.ppEnabledLayerNames;
        dvi.enabledExtensionCount = vdvi.enabledExtensionCount;
        dvi.ppEnabledExtensionNames = vdvi.ppEnabledExtensionNames;
        dvi.pEnabledFeatures = vdvi.pEnabledFeatures;
        if (vkCreateDevice(*(vtDevice->_physicalDevice.lock()), &dvi, nullptr, &vtDevice->_device) == VK_SUCCESS) { result = VT_SUCCESS; };

        VmaAllocatorCreateInfo allocatorInfo;
        allocatorInfo.physicalDevice = *(vtDevice->_physicalDevice.lock());
        allocatorInfo.device = vtDevice->_device;
        allocatorInfo.preferredLargeHeapBlockSize = 16384; // 16kb
        allocatorInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;
        if (vmaCreateAllocator(&allocatorInfo, &vtDevice->_allocator) == VK_SUCCESS) { result = VT_SUCCESS; };

        return result;
    };

    inline auto createDeviceBuffer(std::shared_ptr<Device> device, VkBufferUsageFlagBits usageFlag, VkDeviceSize bufferSize = sizeof(uint32_t), uint32_t familyIndex = 0){
        auto vtDeviceBuffer = std::make_shared<DeviceBuffer>();

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        auto binfo = VkBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, bufferSize, usageFlag, VK_SHARING_MODE_EXCLUSIVE, 1, &familyIndex };
        vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo);
        vtDeviceBuffer->_size = bufferSize;

        return vtDeviceBuffer;
    };

    inline auto createDeviceImage(std::shared_ptr<Device> device, vk::ImageViewType imageViewType, vk::ImageLayout layout, vk::Extent3D size, vk::ImageUsageFlags usage, vk::Format format = vk::Format::eR32G32B32A32Sfloat, uint32_t mipLevels = 1, uint32_t familyIndex = 0) {
        auto texture = std::make_shared<DeviceImage>();
        texture->_layout = (VkImageLayout)layout;

        // init image dimensional type
        vk::ImageType imageType = vk::ImageType::e2D; bool isCubemap = false;
        switch (imageViewType) {
            case vk::ImageViewType::e1D:
                imageType = vk::ImageType::e1D;
                break;
            case vk::ImageViewType::e1DArray:
                imageType = vk::ImageType::e2D;
                break;
            case vk::ImageViewType::e2D:
                imageType = vk::ImageType::e2D;
                break;
            case vk::ImageViewType::e2DArray:
                imageType = vk::ImageType::e3D;
                break;
            case vk::ImageViewType::e3D:
                imageType = vk::ImageType::e3D;
                break;
            case vk::ImageViewType::eCube:
                imageType = vk::ImageType::e3D;
                isCubemap = true;
                break;
            case vk::ImageViewType::eCubeArray:
                imageType = vk::ImageType::e3D;
                isCubemap = true;
                break;
        };

        // image memory descriptor
        auto imageInfo = vk::ImageCreateInfo();
        imageInfo.initialLayout = (vk::ImageLayout)texture->_initialLayout;
        imageInfo.imageType = imageType;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.arrayLayers = 1; // unsupported
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.extent = { size.width, size.height, size.depth * (isCubemap ? 6 : 1) };
        imageInfo.format = format;
        imageInfo.mipLevels = mipLevels;
        imageInfo.pQueueFamilyIndices = &familyIndex;
        imageInfo.queueFamilyIndexCount = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1; // at now not supported MSAA
        imageInfo.usage = usage;

        // create image with allocation
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vmaCreateImage(device->_allocator, &(VkImageCreateInfo)imageInfo, &allocCreateInfo, (VkImage *)&texture->_image, &texture->_allocation, &texture->_allocationInfo);

        // subresource range
        texture->_subresourceRange.levelCount = 1;
        texture->_subresourceRange.layerCount = 1;
        texture->_subresourceRange.baseMipLevel = 0;
        texture->_subresourceRange.baseArrayLayer = 0;
        texture->_subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // subresource layers
        texture->_subresourceLayers.layerCount = texture->_subresourceRange.layerCount;
        texture->_subresourceLayers.baseArrayLayer = texture->_subresourceRange.baseArrayLayer;
        texture->_subresourceLayers.aspectMask = texture->_subresourceRange.aspectMask;
        texture->_subresourceLayers.mipLevel = texture->_subresourceRange.baseMipLevel;

        // descriptor for usage
        texture->_imageView = vk::Device(device->_device).createImageView(vk::ImageViewCreateInfo()
            .setSubresourceRange(texture->_subresourceRange)
            .setViewType(imageViewType)
            .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
            .setImage(texture->_image)
            .setFormat(format));

        return texture;
    };

    // transition texture layout
    inline VkResult imageBarrier(VkCommandBuffer &cmd, std::shared_ptr<DeviceImage> &image) {
        VkResult result = VK_SUCCESS; // planned to complete

        vk::ImageMemoryBarrier imageMemoryBarriers = {};
        imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.oldLayout = (vk::ImageLayout)image->_initialLayout;
        imageMemoryBarriers.newLayout = (vk::ImageLayout)image->_layout;
        imageMemoryBarriers.image = image->_image;
        imageMemoryBarriers.subresourceRange = image->_subresourceRange;

        // Put barrier on top
        vk::PipelineStageFlags srcStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
        vk::PipelineStageFlags dstStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
        vk::DependencyFlags dependencyFlags{};
        vk::AccessFlags srcMask{};
        vk::AccessFlags dstMask{};

        typedef vk::ImageLayout il;
        typedef vk::AccessFlagBits afb;

        // Is it me, or are these the same?
        switch ((vk::ImageLayout)image->_initialLayout) {
            case il::eUndefined: break;
            case il::eGeneral: srcMask = afb::eTransferWrite; break;
            case il::eColorAttachmentOptimal: srcMask = afb::eColorAttachmentWrite; break;
            case il::eDepthStencilAttachmentOptimal: srcMask = afb::eDepthStencilAttachmentWrite; break;
            case il::eDepthStencilReadOnlyOptimal: srcMask = afb::eDepthStencilAttachmentRead; break;
            case il::eShaderReadOnlyOptimal: srcMask = afb::eShaderRead; break;
            case il::eTransferSrcOptimal: srcMask = afb::eTransferRead; break;
            case il::eTransferDstOptimal: srcMask = afb::eTransferWrite; break;
            case il::ePreinitialized: srcMask = afb::eTransferWrite | afb::eHostWrite; break;
            case il::ePresentSrcKHR: srcMask = afb::eMemoryRead; break;
        }

        switch ((vk::ImageLayout)image->_layout) {
            case il::eUndefined: break;
            case il::eGeneral: dstMask = afb::eTransferWrite; break;
            case il::eColorAttachmentOptimal: dstMask = afb::eColorAttachmentWrite; break;
            case il::eDepthStencilAttachmentOptimal: dstMask = afb::eDepthStencilAttachmentWrite; break;
            case il::eDepthStencilReadOnlyOptimal: dstMask = afb::eDepthStencilAttachmentRead; break;
            case il::eShaderReadOnlyOptimal: dstMask = afb::eShaderRead; break;
            case il::eTransferSrcOptimal: dstMask = afb::eTransferRead; break;
            case il::eTransferDstOptimal: dstMask = afb::eTransferWrite; break;
            case il::ePreinitialized: dstMask = afb::eTransferWrite; break;
            case il::ePresentSrcKHR: dstMask = afb::eMemoryRead; break;
        }

        imageMemoryBarriers.srcAccessMask = srcMask;
        imageMemoryBarriers.dstAccessMask = dstMask;

        // barrier
        vk::CommandBuffer(cmd).pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlagBits::eByRegion, {}, {}, std::array<vk::ImageMemoryBarrier, 1>{imageMemoryBarriers});
        image->_initialLayout = (VkImageLayout)imageMemoryBarriers.newLayout;

        return result;
    };

};
