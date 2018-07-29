#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // destructor of DeviceImage
    inline DeviceImage::~DeviceImage() {
        VRT_ASYNC([=]() {
            vmaDestroyImage(_device->_allocator, _image, _allocation);
        });
    };

    static inline VtResult createDeviceImage(std::shared_ptr<Device> device, const VtDeviceImageCreateInfo& cinfo, std::shared_ptr<DeviceImage>& _vtImage) {
        // result will no fully handled
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        auto& texture = (_vtImage = std::make_shared<DeviceImage>());
        texture->_device = device; // delegate device by weak_ptr
        texture->_layout = (VkImageLayout)cinfo.layout;

        // init image dimensional type
        vk::ImageType imageType = vk::ImageType::e2D; bool isCubemap = false;
        switch (vk::ImageViewType(cinfo.imageViewType)) {
            case vk::ImageViewType::e1D: imageType = vk::ImageType::e1D; break;
            case vk::ImageViewType::e1DArray: imageType = vk::ImageType::e2D; break;
            case vk::ImageViewType::e2D: imageType = vk::ImageType::e2D; break;
            case vk::ImageViewType::e2DArray: imageType = vk::ImageType::e3D; break;
            case vk::ImageViewType::e3D: imageType = vk::ImageType::e3D; break;
            case vk::ImageViewType::eCube: imageType = vk::ImageType::e3D; isCubemap = true; break;
            case vk::ImageViewType::eCubeArray: imageType = vk::ImageType::e3D; isCubemap = true; break;
        };

        // additional usage
        auto usage = vk::ImageUsageFlags(cinfo.usage) | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

        // image memory descriptor
        auto imageInfo = vk::ImageCreateInfo();
        imageInfo.initialLayout = vk::ImageLayout(texture->_initialLayout);
        imageInfo.imageType = imageType;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.arrayLayers = 1; // unsupported
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.extent = vk::Extent3D{ cinfo.size.width, cinfo.size.height, cinfo.size.depth * (isCubemap ? 6 : 1) };
        imageInfo.format = vk::Format(cinfo.format);
        imageInfo.mipLevels = cinfo.mipLevels;
        imageInfo.pQueueFamilyIndices = &cinfo.familyIndex;
        imageInfo.queueFamilyIndexCount = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1; // at now not supported MSAA
        imageInfo.usage = usage;

        // create image with allocation
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (vmaCreateImage(device->_allocator, (VkImageCreateInfo*)&imageInfo, &allocCreateInfo, (VkImage *)&texture->_image, &texture->_allocation, &texture->_allocationInfo) == VK_SUCCESS) { result = VK_SUCCESS; };

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

        texture->_extent = imageInfo.extent;

        // descriptor for usage 
        // (unhandled by vtResult)
        texture->_imageView = vk::Device(device->_device).createImageView(vk::ImageViewCreateInfo()
            .setSubresourceRange(texture->_subresourceRange)
            .setViewType(vk::ImageViewType(cinfo.imageViewType))
            .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
            .setImage(texture->_image)
            .setFormat(vk::Format(cinfo.format)));

        return result;
    };

};
