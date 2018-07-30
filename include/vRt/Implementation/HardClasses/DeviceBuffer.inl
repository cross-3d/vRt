#pragma once
#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vrt;

    // destructor of advanced buffer
    template<VmaMemoryUsage U>
    inline RoledBuffer<U>::~RoledBuffer() {
        VRT_ASYNC([=]() {
            vmaDestroyBuffer(_device->_allocator, _buffer, _allocation);
        });
    };

    template<VmaMemoryUsage U>
    static inline VtResult createBuffer(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>>& _vtBuffer) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        auto vtDeviceBuffer = (_vtBuffer = std::make_shared<RoledBuffer<U>>());
        vtDeviceBuffer->_device = device; // delegate device by weak_ptr

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = U;

        // make memory usages 
        auto usageFlag = cinfo.usageFlag;
        if constexpr (U != VMA_MEMORY_USAGE_GPU_ONLY) { allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; }
        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { usageFlag |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; } else {
            if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) { usageFlag |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; } else {
                usageFlag |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                if (cinfo.format) { usageFlag |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; } // if has format, add texel storage usage
            }; // bidirectional
        };

#ifdef VRT_ENABLE_VEZ_INTEROP
        auto binfo = VezBufferCreateInfo{ nullptr, cinfo.bufferSize, usageFlag, 1, &cinfo.familyIndex };
        result = vezCreateBuffer(device->_device, (U - 1), &binfo, &vtDeviceBuffer->_buffer);
#else
        auto binfo = VkBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, cinfo.bufferSize, usageFlag, VK_SHARING_MODE_EXCLUSIVE, 1, &cinfo.familyIndex };
        result = vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo);
#endif

        // if format is known, make bufferView
        if constexpr (U == VMA_MEMORY_USAGE_GPU_ONLY) { // spaghetti code, because had different qualifiers
            if (result == VK_SUCCESS && cinfo.format) {
                vtDeviceBuffer->_bufferView = {};

#ifdef VRT_ENABLE_VEZ_INTEROP
                VezBufferViewCreateInfo bvi = {};
#else
                VkBufferViewCreateInfo bvi = {};
                bvi.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
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

};
