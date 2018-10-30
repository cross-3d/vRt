#pragma once
#include "RTXHandlersDef.inl"
#include "../Backland/vRt_subimpl.inl" // required structures for this part

namespace vrt {
    // passing structure
    class VtRTXAccelerationExtension : public VtDeviceAdvancedAccelerationExtension {
    public:
        friend VtDeviceAdvancedAccelerationExtension;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };


    };
};
