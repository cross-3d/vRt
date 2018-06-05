#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // make default descriptor info for buffers
    inline VkDescriptorBufferInfo DeviceBuffer::_descriptorInfo() {
        return VkDescriptorBufferInfo(vk::DescriptorBufferInfo(_buffer, 0u, _size));
    };

};
