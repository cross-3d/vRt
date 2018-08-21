#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vrt;

    VtResult convertPhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& vtPhysicalDevice) {
        vtPhysicalDevice = std::make_shared<PhysicalDevice>();
        vtPhysicalDevice->_physicalDevice = physical; // assign a Vulkan physical device
        return VK_SUCCESS;
    };
};
