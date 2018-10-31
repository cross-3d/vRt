#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;


    // destructor of roled buffer
    template<VtMemoryUsage U>
    RoledBuffer<U>::~RoledBuffer() {
        auto buffer = this->_buffer; this->_buffer = {};
        auto device = this->_device; this->_device = {};
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        auto allocation = this->_allocation; this->_allocation = {};
#endif
        std::async([=](){
            if (buffer && device)
#ifdef VRT_ENABLE_VEZ_INTEROP
                vezDestroyBuffer(device->_device, buffer);
#else
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
                if (allocation) vmaDestroyBuffer(device->_allocator, buffer, allocation);
#endif
#endif
        });
    };


    BufferRegion::~BufferRegion() {
        if (_bufferView() && _device) vkDestroyBufferView(*_device, _bufferView(), nullptr); _bufferView() = {}, _device = {};
    };


    VtResult createBufferView(std::shared_ptr<BufferRegion> bRegion) {
        VtResult result = VK_SUCCESS;

        if (bRegion->_size() < sizeof(uint32_t)) result = VK_ERROR_INITIALIZATION_FAILED;
        if (result == VK_SUCCESS && bRegion->_format && bRegion->_device) {
            const auto device = bRegion->_device;

#ifdef VRT_ENABLE_VEZ_INTEROP
            auto bvi = VezBufferViewCreateInfo{};
#else
            auto bvi = VkBufferViewCreateInfo(vk::BufferViewCreateInfo{});
            bvi.flags = {};
#endif
            bvi.pNext = nullptr;
            bvi.buffer = bRegion->_descriptorInfo().buffer;
            bvi.format = bRegion->_format;
            bvi.offset = bRegion->_offset();
            bvi.range = bRegion->_size();

            if (bRegion->_size() > 0) {
#ifdef VRT_ENABLE_VEZ_INTEROP
                if (vezCreateBufferView(device->_device, &bvi, &bRegion->_bufferView()) == VK_SUCCESS) {
#else
                if (vkCreateBufferView(device->_device, &bvi, nullptr, &bRegion->_bufferView()) == VK_SUCCESS) {
#endif
                    result = VK_SUCCESS;
                }
                else {
                    result = VK_INCOMPLETE;
                };
            };
        };
        return result;
    };


    template<VtMemoryUsage U>
    VtResult createBuffer(std::shared_ptr<Device> device, VtDeviceBufferCreateInfo cinfo, std::shared_ptr<RoledBuffer<U>>& vtDeviceBuffer) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        //auto vtDeviceBuffer = (_vtBuffer = std::make_shared<RoledBuffer<U>>());
        vtDeviceBuffer = std::make_shared<RoledBuffer<U>>();
        vtDeviceBuffer->_device = device; // delegate device by weak_ptr

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VmaMemoryUsage(U); // TODO: stable conversion

        // make memory usages 
        auto usageFlagCstr = 0u;
        if constexpr (U != VT_MEMORY_USAGE_GPU_ONLY) { allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; }
        if constexpr (U == VT_MEMORY_USAGE_CPU_TO_GPU) { usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; }
        else {
            if constexpr (U == VT_MEMORY_USAGE_GPU_TO_CPU) { usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; }
            else {
                usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            };
        };

        auto usageFlag = cinfo.usageFlag | usageFlagCstr;
        //if (cinfo.format) { usageFlag |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; } // if has format, add texel storage usage
        usageFlag |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

#ifdef VRT_ENABLE_VEZ_INTEROP
        VezMemoryFlags mem = VEZ_MEMORY_GPU_ONLY;
        if constexpr (U == VT_MEMORY_USAGE_CPU_TO_GPU) mem = VEZ_MEMORY_CPU_TO_GPU;
        if constexpr (U == VT_MEMORY_USAGE_GPU_TO_CPU) mem = VEZ_MEMORY_GPU_TO_CPU;
        if constexpr (U == VT_MEMORY_USAGE_CPU_ONLY) mem = VEZ_MEMORY_CPU_ONLY;

        auto binfo = VezBufferCreateInfo{};
        binfo.pNext = nullptr;
#else
        auto binfo = VkBufferCreateInfo(vk::BufferCreateInfo{});
        binfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
#endif

        binfo.queueFamilyIndexCount = 1;
        binfo.pQueueFamilyIndices = &device->_mainFamilyIndex;//&cinfo.familyIndex;

        binfo.size = cinfo.bufferSize;//((cinfo.bufferSize >> 5ull) << 5ull) + 32ull;
        binfo.usage = usageFlag;

        if (binfo.size > 0) {
#ifdef VRT_ENABLE_VEZ_INTEROP
            result = vezCreateBuffer(device->_device, mem, &binfo, &vtDeviceBuffer->_buffer);
#else
            result = vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo);
#endif
        };

        // if format is known, make bufferView
        if constexpr (U == VT_MEMORY_USAGE_GPU_ONLY) {
            VtBufferRegionCreateInfo rbc = {};
            rbc.offset = 0, rbc.bufferSize = cinfo.bufferSize, rbc.format = cinfo.format;
            createBufferRegion(vtDeviceBuffer, rbc, vtDeviceBuffer->_bufferRegion);
        };

        return result;
    };


    // create shared buffer, buffer views and resolve descriptor info (with externalization support)
    inline VtResult createSharedBuffer(std::shared_ptr<BufferManager> bManager, VtDeviceBufferCreateInfo cinfo, std::shared_ptr<DeviceBuffer>& gBuffer) {
        cinfo.bufferSize = bManager->_size; createDeviceBuffer(bManager->_device, cinfo, bManager->_bufferStore); gBuffer = bManager->_bufferStore;

        // complete descriptors and buffer-views
        bManager->_bufferStore->_sharedRegions = bManager->_bufferRegions; // link regions with buffer
        const auto wptr = std::weak_ptr(bManager->_bufferStore);
        for (auto f : bManager->_bufferRegions) {
            f->_boundBuffer = wptr, f->_descriptorInfo().buffer = VkBuffer(*bManager->_bufferStore); createBufferView(f);
        }

        // return result (TODO: handling)
        return VK_SUCCESS;
    };


    // create shared buffer, buffer views and resolve descriptor info
    inline VtResult createSharedBuffer(std::shared_ptr<BufferManager> bManager, VtDeviceBufferCreateInfo cinfo) {
        cinfo.bufferSize = bManager->_size; createDeviceBuffer(bManager->_device, cinfo, bManager->_bufferStore);

        // complete descriptors and buffer-views
        bManager->_bufferStore->_sharedRegions = bManager->_bufferRegions; // link regions with buffer
        const auto wptr = std::weak_ptr(bManager->_bufferStore);
        for (auto f : bManager->_bufferRegions) {
            f->_boundBuffer = wptr, f->_descriptorInfo().buffer = VkBuffer(*bManager->_bufferStore); createBufferView(f);
        };

        // return result (TODO: handling)
        return VK_SUCCESS;
    };


    // create buffer manager
    inline VtResult createBufferManager(std::shared_ptr<Device> gDevice, std::shared_ptr<BufferManager>& bManager) {
        bManager = std::make_shared<BufferManager>();
        bManager->_device = gDevice;
        return VK_SUCCESS;
    };


    // create buffer region by exist buffer
    inline VtResult createBufferRegion(std::shared_ptr<DeviceBuffer> gBuffer, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion) {
        const auto correctedSize = bri.bufferSize;//((bri.bufferSize >> 5ull) << 5ull) + 32ull;
        bRegion = std::make_shared<BufferRegion>();
        bRegion->_device = gBuffer->_device;
        bRegion->_format = bri.format;
        bRegion->_descriptorInfo().range = correctedSize;
        bRegion->_descriptorInfo().offset = bri.offset;
        bRegion->_descriptorInfo().buffer = *(bRegion->_boundBuffer = std::weak_ptr(gBuffer)).lock(); createBufferView(bRegion);
        gBuffer->_sharedRegions.push_back(bRegion); // add shared buffer region
        return VK_SUCCESS;
    };


    // create buffer region by exist buffer
    inline VtResult createBufferRegion(VkBuffer vkBuffer, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion, std::shared_ptr<Device> gDevice) {
        const auto gBuffer = std::make_shared<DeviceBuffer>(); gBuffer->_device = gDevice;
        const auto correctedSize = bri.bufferSize;//((bri.bufferSize >> 5ull) << 5ull) + 32ull;
        bRegion = std::make_shared<BufferRegion>();
        bRegion->_device = gDevice;
        bRegion->_format = bri.format;
        bRegion->_descriptorInfo().range = correctedSize;
        bRegion->_descriptorInfo().offset = bri.offset;
        bRegion->_descriptorInfo().buffer = vkBuffer; createBufferView(bRegion);
        gBuffer->_sharedRegions.push_back(bRegion); // add shared buffer region
        return VK_SUCCESS;
    };


    // create structuring 
    inline VtResult BufferManager::_prealloc(VtBufferRegionCreateInfo cinfo, std::shared_ptr<BufferRegion>& bRegion) {
        const VkDeviceSize correctedSize = ((cinfo.bufferSize >> 5ull) << 5ull) + 32ull, offset = this->_size; this->_size += correctedSize;
        _bufferRegions.push_back(std::make_shared<BufferRegion>());
        bRegion = _bufferRegions[_bufferRegions.size() - 1];
        bRegion->_device = _device;
        bRegion->_format = cinfo.format;
        bRegion->_descriptorInfo().range = correctedSize;
        bRegion->_descriptorInfo().offset = offset;
        return VK_SUCCESS;
    };

    // create buffer region by buffer manager
    inline VtResult createBufferRegion(std::shared_ptr<BufferManager> bManager, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion) {
        return bManager->_prealloc(bri, bRegion);
    };



    // destructor of DeviceImage
    DeviceImage::~DeviceImage() {
        auto  image = this->_image ; this->_image  = {};
        auto device = this->_device; this->_device = {};
        if (_imageView && device) vkDestroyImageView(*device, _imageView, nullptr); _imageView = {};
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        auto allocation = this->_allocation; this->_allocation = {};
#endif
        std::async([=](){
            if (image && device)
#ifdef VRT_ENABLE_VEZ_INTEROP
                vezDestroyImage(device->_device, image);
#else
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
                if (allocation) vmaDestroyImage(device->_allocator, image, allocation);
#endif
#endif
        });
    };

    VtResult createDeviceImage(std::shared_ptr<Device> device, VtDeviceImageCreateInfo cinfo, std::shared_ptr<DeviceImage>& vtDeviceImage) {
        // result will no fully handled
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        //auto vtDeviceImage = (_vtImage = std::make_shared<DeviceImage>());
        vtDeviceImage = std::make_shared<DeviceImage>();
        vtDeviceImage->_device = device; // delegate device by weak_ptr
        vtDeviceImage->_layout = (VkImageLayout)cinfo.layout;
        vtDeviceImage->_initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;

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
        //auto usage = vk::ImageUsageFlags(cinfo.usage) | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
        auto usage = cinfo.usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        // image memory descriptor
#ifdef VRT_ENABLE_VEZ_INTEROP
        auto imageInfo = VezImageCreateInfo{};
#else
        auto imageInfo = VkImageCreateInfo(vk::ImageCreateInfo{});
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.initialLayout = VkImageLayout(vtDeviceImage->_initialLayout);
        imageInfo.sharingMode = VkSharingMode(vk::SharingMode::eExclusive);
#endif

        // unified create structure
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VkImageType(imageType);
        imageInfo.arrayLayers = 1; // unsupported
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.extent = VkExtent3D{ cinfo.size.width, cinfo.size.height, cinfo.size.depth * (isCubemap ? 6 : 1) };
        imageInfo.format = cinfo.format;
        imageInfo.mipLevels = cinfo.mipLevels;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.usage = usage;

        imageInfo.queueFamilyIndexCount = 1;
        imageInfo.pQueueFamilyIndices = &device->_mainFamilyIndex;//&cinfo.familyIndex;

        

        // create image with allocation
#ifdef VRT_ENABLE_VEZ_INTEROP
        VezMemoryFlags mem = VEZ_MEMORY_GPU_ONLY;
        if ( vezCreateImage(device->_device, mem, &imageInfo, &vtDeviceImage->_image) == VK_SUCCESS ) { result = VK_SUCCESS; };
#else
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if ( vmaCreateImage(device->_allocator, &imageInfo, &allocCreateInfo, &vtDeviceImage->_image, &vtDeviceImage->_allocation, &vtDeviceImage->_allocationInfo) == VK_SUCCESS ) { result = VK_SUCCESS; };
#endif


        // subresource range
        vtDeviceImage->_subresourceRange.levelCount = 1;
        vtDeviceImage->_subresourceRange.layerCount = 1;
        vtDeviceImage->_subresourceRange.baseMipLevel = 0;
        vtDeviceImage->_subresourceRange.baseArrayLayer = 0;
        vtDeviceImage->_subresourceRange.aspectMask = cinfo.aspect; //VK_IMAGE_ASPECT_COLOR_BIT;

        // subresource layers
        vtDeviceImage->_subresourceLayers.layerCount = vtDeviceImage->_subresourceRange.layerCount;
        vtDeviceImage->_subresourceLayers.baseArrayLayer = vtDeviceImage->_subresourceRange.baseArrayLayer;
        vtDeviceImage->_subresourceLayers.aspectMask = vtDeviceImage->_subresourceRange.aspectMask;
        vtDeviceImage->_subresourceLayers.mipLevel = vtDeviceImage->_subresourceRange.baseMipLevel;
        vtDeviceImage->_extent = imageInfo.extent;


        // image view for usage
#ifdef VRT_ENABLE_VEZ_INTEROP
        auto vinfo = VezImageViewCreateInfo{};
        vinfo.subresourceRange = *(VezImageSubresourceRange*)(&vtDeviceImage->_subresourceRange.baseMipLevel);
#else
        auto vinfo = VkImageViewCreateInfo(vk::ImageViewCreateInfo{});
        vinfo.subresourceRange = vtDeviceImage->_subresourceRange;
        vinfo.flags = {};
#endif
        vinfo.pNext = nullptr;
        vinfo.components = VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        vinfo.format = cinfo.format;
        vinfo.image = vtDeviceImage->_image;
        vinfo.viewType = cinfo.imageViewType;

#ifdef VRT_ENABLE_VEZ_INTEROP
        vezCreateImageView(device->_device, &vinfo, &vtDeviceImage->_imageView);
        vtDeviceImage->_initialLayout = vtDeviceImage->_layout;
#else
        vtDeviceImage->_imageView = vk::Device(device->_device).createImageView(vk::ImageViewCreateInfo(vinfo));
#endif

        // anyways create static descriptor
        vtDeviceImage->_sDescriptorInfo = VkDescriptorImageInfo{ {}, vtDeviceImage->_imageView, vtDeviceImage->_layout };
        return result;
    };

};
