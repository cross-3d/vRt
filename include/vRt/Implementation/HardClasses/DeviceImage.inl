#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // make default descriptor info for images
    inline VkDescriptorImageInfo DeviceImage::_descriptorInfo() const {
        return VkDescriptorImageInfo(vk::DescriptorImageInfo(nullptr, _imageView, vk::ImageLayout(_layout)));
    };

    // destructor of DeviceImage
    inline DeviceImage::~DeviceImage() {
        vmaDestroyImage(_device.lock()->_allocator, _image, _allocation);
    };

};
