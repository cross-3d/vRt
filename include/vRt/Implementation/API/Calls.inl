#pragma once

#include "../../vRt_subimpl.inl"
#include "./BuildAccelerator.inl"
#include "./RayTracing.inl"

namespace _vt {
    using namespace vt;

    // experimental implementation of "vtCreateDevice"
    inline VtResult vtCreateDevice(VtPhysicalDevice vtPhysicalDevice, const VtDeviceCreateInfo * vtDeviceCreateInfo, VtDevice * vtDevice) {
        vtDevice->_vtDevice = createDevice(vtPhysicalDevice._vtPhysicalDevice, *vtDeviceCreateInfo); // don't know, will removed, or not
    };
};
