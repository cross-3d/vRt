#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXExtendedAPI.inl"
#include "../RTXClasses.inl"

namespace vrt {

    // make sure that RTX really supported
    VtResult VtRTXAcceleratorExtension::_Criteria(std::shared_ptr<_vt::DeviceFeatures> lwFeatures) const {
        const auto rayTracingNV = "VK_NV_ray_tracing"; // awaiting support of extension
        if (lwFeatures->_features.features.shaderInt16) {
            for (auto i : lwFeatures->_extensions) {
                if (std::string(i.extensionName).compare(rayTracingNV) == 0) return VK_SUCCESS; // RTX have support
            };
        };
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

    // initialize RTX extension
    VtResult VtRTXAcceleratorExtension::_Initialization(std::shared_ptr<_vt::Device> lwDevice, std::shared_ptr<_vt::AcceleratorExtensionBase>& _hExtensionAccelerator) const {
        auto accelerationExtension = std::make_shared<_vt::RTXAcceleratorExtension>();
        _hExtensionAccelerator = std::dynamic_pointer_cast<_vt::AcceleratorExtensionBase>(accelerationExtension);
        return accelerationExtension->_Init(lwDevice, this);
    };

    // required for RTX top level support 
    VtResult vtGetAcceleratorHandleNV(VtAcceleratorSet accSet, VtHandleRTX * acceleratorHandleNV) {
        if (accSet->_hExtension && accSet->_hExtension->_AccelerationName() == VT_ACCELERATION_NAME_RTX) {
            return vkGetAccelerationStructureHandleNV(VkDevice(*accSet->_device), std::dynamic_pointer_cast<_vt::RTXAcceleratorSetExtension>(accSet->_hExtension)->_accelStructureNV, sizeof(VtHandleRTX), acceleratorHandleNV);
        };
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

};
