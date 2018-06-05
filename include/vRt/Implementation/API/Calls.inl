#pragma once

#include "../../vRt_subimpl.inl"
#include "./BuildAccelerator.inl"
#include "./RayTracing.inl"

namespace _vt {
    using namespace vt;

    // experimental implementation of "vtCreateDevice"
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice) {
        return createDevice(vtPhysicalDevice._vtPhysicalDevice, *deviceCreateInfo, *vtDevice);
    };

    inline VtResult vtCmdImageBarrier(VkCommandBuffer cmd, VtDeviceImage image) {
        return imageBarrier(cmd, image._vtDeviceImage);
    };
};
