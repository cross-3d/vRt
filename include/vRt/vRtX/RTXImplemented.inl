#pragma once

#include "RTXAcceleratorExtension.hpp"
#include "RTXClasses.inl"
#include "Implementation/RTXAccelerator.inl"
#include "Implementation/RTXAcceleratorSet.inl"
#include "Implementation/RTXVertexAssemblySet.inl"

namespace vrt {

     VtResult VtRTXAcceleratorExtension::_Criteria(std::shared_ptr<_vt::DeviceFeatures> lwFeatures) const {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };
     VtResult VtRTXAcceleratorExtension::_Initialization(std::shared_ptr<_vt::Device> lwDevice, std::shared_ptr<_vt::AdvancedAcceleratorBase>& _hExtensionAccelerator) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };
    
};
