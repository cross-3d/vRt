#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // make default descriptor info for images
    inline VkDescriptorImageInfo DeviceImage::_descriptorInfo() {
        return VkDescriptorImageInfo(vk::DescriptorImageInfo(nullptr, _imageView, vk::ImageLayout(_layout)));
    };

};
