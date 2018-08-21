#pragma once
#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vrt;

    // destructor of advanced buffer
    template<VmaMemoryUsage U>
    RoledBuffer<U>::~RoledBuffer() {
        std::async([=]() {
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezDestroyBuffer(_device->_device, _buffer);
#else
            vmaDestroyBuffer(_device->_allocator, _buffer, _allocation);
#endif
        });
    };


    VtResult createBufferView(std::shared_ptr<Device> device, std::shared_ptr<BufferRegion>& bRegion) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;
        if (result == VK_SUCCESS && bRegion->_format) {
            bRegion->_bufferView = {};

#ifdef VRT_ENABLE_VEZ_INTEROP
            auto bvi = VezBufferViewCreateInfo{};
#else
            auto bvi = VkBufferViewCreateInfo(vk::BufferViewCreateInfo{});
            bvi.flags = {};
#endif
            bvi.pNext = nullptr;
            bvi.buffer = VkBuffer(*bRegion);
            bvi.format = bRegion->_format;
            bvi.offset = 0;
            bvi.range = VK_WHOLE_SIZE;

#ifdef VRT_ENABLE_VEZ_INTEROP
            if (vezCreateBufferView(device->_device, &bvi, &vtDeviceBuffer->_bufferView) == VK_SUCCESS) {
#else
            if (vkCreateBufferView(device->_device, &bvi, nullptr, &bRegion->_bufferView) == VK_SUCCESS) {
#endif
                result = VK_SUCCESS;
            }
            else {
                result = VK_INCOMPLETE;
            };
        };
        return result;
    };


    template<VmaMemoryUsage U>
    VtResult createBuffer(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>>& vtDeviceBuffer) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        //auto vtDeviceBuffer = (_vtBuffer = std::make_shared<RoledBuffer<U>>());
        vtDeviceBuffer = std::make_shared<RoledBuffer<U>>();
        vtDeviceBuffer->_device = device; // delegate device by weak_ptr

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = U;

        // make memory usages 
        auto usageFlagCstr = 0u;
        if constexpr (U != VMA_MEMORY_USAGE_GPU_ONLY) { allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; }
        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; } else {
            if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) { usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; } else {
                usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            };
        };

        auto usageFlag = cinfo.usageFlag | usageFlagCstr;
        if (cinfo.format) { usageFlag |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; } // if has format, add texel storage usage

#ifdef VRT_ENABLE_VEZ_INTEROP
        VezMemoryFlags mem = VEZ_MEMORY_GPU_ONLY;
        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) mem = VEZ_MEMORY_CPU_TO_GPU;
        if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) mem = VEZ_MEMORY_GPU_TO_CPU;
        if constexpr (U == VMA_MEMORY_USAGE_CPU_ONLY) mem = VEZ_MEMORY_CPU_ONLY;

        auto binfo = VezBufferCreateInfo{};
        binfo.pNext = nullptr;
#else
        auto binfo = VkBufferCreateInfo(vk::BufferCreateInfo{});
        binfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
#endif
        //binfo.queueFamilyIndexCount = 1;
        //binfo.pQueueFamilyIndices = &cinfo.familyIndex;
        binfo.size = cinfo.bufferSize;
        binfo.usage = usageFlag;

#ifdef VRT_ENABLE_VEZ_INTEROP
        result = vezCreateBuffer(device->_device, mem, &binfo, &vtDeviceBuffer->_buffer);
#else
        result = vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo);
#endif


        // if format is known, make bufferView
        // ON DEPRECATION
        if constexpr (U == VMA_MEMORY_USAGE_GPU_ONLY) { // spaghetti code, because had different qualifiers
            if (result == VK_SUCCESS && cinfo.format) {
                vtDeviceBuffer->_bufferView = {};

#ifdef VRT_ENABLE_VEZ_INTEROP
                auto bvi = VezBufferViewCreateInfo{};
#else
                auto bvi = VkBufferViewCreateInfo(vk::BufferViewCreateInfo{});
                bvi.flags = {};
#endif
                bvi.pNext = nullptr;
                bvi.buffer = vtDeviceBuffer->_buffer;
                bvi.format = cinfo.format;
                bvi.offset = 0;
                bvi.range = VK_WHOLE_SIZE;

#ifdef VRT_ENABLE_VEZ_INTEROP
                if (vezCreateBufferView(device->_device, &bvi, &vtDeviceBuffer->_bufferView) == VK_SUCCESS) {
#else
                if (vkCreateBufferView(device->_device, &bvi, nullptr, &vtDeviceBuffer->_bufferView) == VK_SUCCESS) {
#endif
                    result = VK_SUCCESS;
                } else {
                    result = VK_INCOMPLETE;
                };
            }
        }
        

        vtDeviceBuffer->_size = cinfo.bufferSize;
        vtDeviceBuffer->_staticDsci = VkDescriptorBufferInfo{ vtDeviceBuffer->_buffer, 0u, VK_WHOLE_SIZE };
        return result;
    };


    // create shared buffer, buffer views and resolve descriptor info
    VtResult createSharedBuffer(const std::shared_ptr<BufferManager>& bManager, VtDeviceBufferCreateInfo cinfo) {
        cinfo.bufferSize = bManager->_size; createDeviceBuffer(bManager->_device, cinfo, bManager->_bufferStore);

        // complete descriptors and buffer-views
        for (auto&f : bManager->_bufferRegions) {
            f->_descriptorInfo.buffer = *(f->_boundBuffer = bManager->_bufferStore);
            createBufferView(bManager->_device, f);
        }

        // return result (TODO: handling)
        return VK_SUCCESS;
    };


};
