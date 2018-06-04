#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vt;



    uint32_t VtIdentifier = 0x1FFu;
    struct VkShortHead {
        //uint32_t sublevel : 24, identifier : 12;
        uint32_t sublevel : 12, identifier : 24; // I don't know ordering of bits
        uintptr_t pNext; // pull of C++20 raw pointers
    };

    struct VkFullHead {
        uint32_t sType;
        uintptr_t pNext; // pull of C++20 raw pointers
    };


    template <class VtS>
    auto vtSearchStructure(VtS& structure, VtStructureType sType) {
        VkFullHead* head = (VkFullHead*)&structure;
        VkFullHead* found = nullptr;
        for (int i = 0; i < 255; i++) {
            if (!head) break;
            if (head->sType == sType) {
                found = head; break;
            }
            head = (VkFullHead*)head->pNext;
        }
        return found;
    };

    template <class VtS>
    auto vtExplodeArtificals(VtS& structure) {
        VkShortHead* head = (VkShortHead*)&structure;
        VkShortHead* lastVkStructure = nullptr;
        VkShortHead* firstVkStructure = nullptr;
        for (int i = 0; i < 255; i++) {
            if (!head) break;
            if (head->identifier != VtIdentifier) {
                if (lastVkStructure) {
                    lastVkStructure->pNext = (uintptr_t)head;
                }
                else {
                    firstVkStructure = head;
                }
                lastVkStructure = head;
            }
            head = (VkShortHead*)head->pNext;
        }
        return firstVkStructure;
    };


    inline VtResult makePhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, VtPhysicalDevice& _vtPhysicalDevice){
        _vtPhysicalDevice._vtPhysicalDevice = std::make_shared<PhysicalDevice>();
        _vtPhysicalDevice._vtPhysicalDevice->_physicalDevice = physical; // assign a Vulkan physical device
        return VK_SUCCESS;
    };

    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo& vdvi, VtDevice& _vtDevice){
        auto& vtDevice = (_vtDevice._vtDevice = std::make_shared<Device>());
        vtDevice->_physicalDevice = physicalDevice; // reference for aliasing

        VtResult result = VK_ERROR_INITIALIZATION_FAILED;
        VtArtificalDeviceExtension vtExtension; // default structure values
        auto vtExtensionPtr = vtSearchStructure(vdvi, VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION);
        if (vtExtensionPtr) { // if found, getting some info
            vtExtension = (VtArtificalDeviceExtension&)*vtExtensionPtr;
        }

        // be occurate with "VkDeviceCreateInfo", because after creation device, all "vt" extended structures will destoyed
        if (vkCreateDevice(*(vtDevice->_physicalDevice.lock()), (const VkDeviceCreateInfo*)vtExplodeArtificals(vdvi), nullptr, &vtDevice->_device) == VK_SUCCESS) { result = VK_SUCCESS; };

        VmaAllocatorCreateInfo allocatorInfo;
        allocatorInfo.physicalDevice = *(vtDevice->_physicalDevice.lock());
        allocatorInfo.device = vtDevice->_device;
        allocatorInfo.preferredLargeHeapBlockSize = 16384; // 16kb
        allocatorInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;
        if (vmaCreateAllocator(&allocatorInfo, &vtDevice->_allocator) == VK_SUCCESS) { result = VK_SUCCESS; };

        return result;
    };

    // planned acceptance by VtCreateDeviceBufferInfo 
    inline VtResult createDeviceBuffer(std::shared_ptr<Device> device, VtDeviceBufferCreateInfo cinfo, VtDeviceBuffer &_vtBuffer){
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        auto& vtDeviceBuffer = (_vtBuffer._deviceBuffer = std::make_shared<DeviceBuffer>());
        vtDeviceBuffer->_device = device; // delegate device by weak_ptr

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        auto binfo = VkBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, cinfo.bufferSize, cinfo.usageFlag, VK_SHARING_MODE_EXCLUSIVE, 1, &cinfo.familyIndex };
        if (vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo) == VK_SUCCESS) { result = VK_SUCCESS; };
        vtDeviceBuffer->_size = cinfo.bufferSize;

        // if format is known, make bufferView
        if (result == VK_SUCCESS && cinfo.format) {
            vtDeviceBuffer->_bufferView;
            VkBufferViewCreateInfo bvi;
            bvi.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            bvi.buffer = vtDeviceBuffer->_buffer;
            bvi.format = cinfo.format;
            bvi.offset = 0;
            bvi.range = cinfo.bufferSize;
            if (vkCreateBufferView(device->_device, &bvi, nullptr, &vtDeviceBuffer->_bufferView) == VK_SUCCESS) {
                result = VK_SUCCESS;
            }
            else {
                result = VK_INCOMPLETE;
            };
        }

        return result;
    };

    // planned acceptance by VtCreateDeviceImageInfo 
    inline VtResult createDeviceImage(std::shared_ptr<Device> device, VtDeviceImageCreateInfo cinfo, VtDeviceImage &_vtImage) {
        // result will no fully handled
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        auto& texture = (_vtImage._deviceImage = std::make_shared<DeviceImage>());
        texture->_device = device; // delegate device by weak_ptr
        texture->_layout = (VkImageLayout)cinfo.layout;

        // init image dimensional type
        vk::ImageType imageType = vk::ImageType::e2D; bool isCubemap = false;
        switch (vk::ImageViewType(cinfo.imageViewType)) {
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
        imageInfo.initialLayout = vk::ImageLayout(texture->_initialLayout);
        imageInfo.imageType = imageType;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.arrayLayers = 1; // unsupported
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.extent = { cinfo.size.width, cinfo.size.height, cinfo.size.depth * (isCubemap ? 6 : 1) };
        imageInfo.format = vk::Format(cinfo.format);
        imageInfo.mipLevels = cinfo.mipLevels;
        imageInfo.pQueueFamilyIndices = &cinfo.familyIndex;
        imageInfo.queueFamilyIndexCount = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1; // at now not supported MSAA
        imageInfo.usage = vk::ImageUsageFlags(cinfo.usage);

        // create image with allocation
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (vmaCreateImage(device->_allocator, &(VkImageCreateInfo)imageInfo, &allocCreateInfo, (VkImage *)&texture->_image, &texture->_allocation, &texture->_allocationInfo) == VK_SUCCESS) { result = VK_SUCCESS; };

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
        // (unhandled by vtResult)
        texture->_imageView = vk::Device(device->_device).createImageView(vk::ImageViewCreateInfo()
            .setSubresourceRange(texture->_subresourceRange)
            .setViewType(vk::ImageViewType(cinfo.imageViewType))
            .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
            .setImage(texture->_image)
            .setFormat(vk::Format(cinfo.format)));

        return result;
    };

    // transition texture layout
    inline VtResult imageBarrier(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> image) {
        VtResult result = VK_SUCCESS; // planned to complete

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

        // assign access masks
        imageMemoryBarriers.srcAccessMask = srcMask;
        imageMemoryBarriers.dstAccessMask = dstMask;

        // barrier
        vk::CommandBuffer(cmd).pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlagBits::eByRegion, {}, {}, std::array<vk::ImageMemoryBarrier, 1>{imageMemoryBarriers});
        image->_initialLayout = (VkImageLayout)imageMemoryBarriers.newLayout;

        return result;
    };

};
