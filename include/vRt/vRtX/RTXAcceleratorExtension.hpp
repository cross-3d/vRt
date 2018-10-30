#pragma once
#include "../vRt.hpp"
#include "RTXAcceleratorExtension.hpp"
#include "RTXHandlers.inl"

namespace vrt {
    
    class VtRTXAcceleratorExtension : VtDeviceAdvancedAccelerationExtension {
        protected: // API in-runtime implementation (dynamic polymorphism)
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; }; // in-runtime return acceleration extension name
        virtual VtResult _Criteria(std::shared_ptr<_vt::DeviceFeatures> lwFeatures) const override {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        };
        virtual VtResult _Initialization(std::shared_ptr<_vt::Device> lwDevice, std::shared_ptr<_vt::AdvancedAcceleratorBase>& _hExtensionAccelerator) override {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        };

        public: 
        
    };

};
