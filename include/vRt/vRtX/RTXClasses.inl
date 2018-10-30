#pragma once

#include "RTXClassesDef.inl"
//#include "../Backland/vRt_subimpl.inl"
//#include "../Backland/Definitions/Structures.inl"
#include "../Backland/Definitions/HardClasses.inl"

namespace _vt {

    // 
    class RTXAcceleratorSetExtension : public AcceleratorSetExtensionBase, std::enable_shared_from_this<RTXAcceleratorSetExtension> {
    public:
        friend Device;
        friend AcceleratorSetExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

    };

    // 
    class RTXAccelerator : public AdvancedAcceleratorBase, std::enable_shared_from_this<RTXAccelerator> {
    public:
        friend Device;
        friend AdvancedAcceleratorBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

    };

};
