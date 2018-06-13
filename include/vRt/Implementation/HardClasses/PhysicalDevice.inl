#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    inline VtResult convertPhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& _vtPhysicalDevice) {
        _vtPhysicalDevice = std::make_shared<PhysicalDevice>();
        _vtPhysicalDevice->_physicalDevice = physical; // assign a Vulkan physical device
        return VK_SUCCESS;
    };
};
