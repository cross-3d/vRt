#pragma once

#include "../../vRt_subimpl.inl"
#include "./BuildAccelerator.inl"
#include "./RayTracing.inl"
#include "./RadixSort.inl"

namespace _vt {
    using namespace vt;

    // TODO to complete

    // experimental implementation of "vtCreateDevice"
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, VkDeviceCreateInfo * deviceCreateInfo, VtDevice * vtDevice) {
        return createDevice(vtPhysicalDevice._vtPhysicalDevice, *deviceCreateInfo, vtDevice->_vtDevice);
    };

    inline VtResult vtCmdImageBarrier(VkCommandBuffer cmd, VtDeviceImage image) {
        return imageBarrier(cmd, image._vtDeviceImage);
    };
};
