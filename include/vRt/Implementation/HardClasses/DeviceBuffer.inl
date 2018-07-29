#pragma once
#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

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

        auto& vtDeviceBuffer = (_vtBuffer = std::make_shared<RoledBuffer<U>>());
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

        auto binfo = VkBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, cinfo.bufferSize, usageFlag, VK_SHARING_MODE_EXCLUSIVE, 1, &cinfo.familyIndex };
        if (vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo) == VK_SUCCESS) { result = VK_SUCCESS; };
        vtDeviceBuffer->_size = cinfo.bufferSize;

        // if format is known, make bufferView
        if constexpr (U == VMA_MEMORY_USAGE_GPU_ONLY) { // spaghetti code, because had different qualifiers
            if (result == VK_SUCCESS && cinfo.format) {
                vtDeviceBuffer->_bufferView;
                VkBufferViewCreateInfo bvi;
                bvi.pNext = nullptr;
                bvi.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
                bvi.flags = {};
                bvi.buffer = vtDeviceBuffer->_buffer;
                bvi.format = cinfo.format;
                bvi.offset = 0;
                bvi.range = VK_WHOLE_SIZE;
                if (vkCreateBufferView(device->_device, &bvi, nullptr, &vtDeviceBuffer->_bufferView) == VK_SUCCESS) {
                    result = VK_SUCCESS;
                } else {
                    result = VK_INCOMPLETE;
                };
            }
        }

        return result;
    };

};
