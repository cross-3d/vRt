#pragma once
#include "vRt/vRt.h"

// planned after RTX 2070 or after 2019 year


// 
namespace _vt {
    class RTXAcceleratorSetExtension;
    class RTXAccelerator;
};


// TODO Turing
namespace vrt {

    // extending to enum type
    constexpr inline static const auto VT_ACCELERATION_NAME_RTX = VtAccelerationName(0x00001000u); // planned in 2019
    class VtRTXAccelerationExtension; // structure extension

};


// lower level hard classes (WIP)
#include "vRt/Backland/Definitions/HardClasses.inl"

namespace _vt {

    class RTXAcceleratorSetExtensionData : public AcceleratorSetExtensionDataBase, std::enable_shared_from_this<RTXAcceleratorSetExtensionData> {
    public:
        friend Device;


    };

    // 
    class RTXAcceleratorSetExtension : public AcceleratorSetExtensionBase, std::enable_shared_from_this<RTXAcceleratorSetExtension> {
    public:
        friend Device;
        friend AcceleratorSetExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

        // accessors of data 
        auto* operator->()  { return dynamic_cast<RTXAcceleratorSetExtensionData*>(_dataPtr.get()); };
        auto* operator->() const  { return dynamic_cast<RTXAcceleratorSetExtensionData*>(_dataPtr.get()); };
    };

    // 
    class RTXAcceleratorData : public AdvancedAcceleratorDataBase, std::enable_shared_from_this<RTXAcceleratorData> {
    public:
        friend Device;


    };

    // 
    class RTXAccelerator : public AdvancedAcceleratorBase, std::enable_shared_from_this<RTXAccelerator> {
    public:
        friend Device;
        friend AdvancedAcceleratorBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

        // accessors of data
        auto* operator->()  { return dynamic_cast<RTXAcceleratorData*>(_dataPtr.get()); };
        auto* operator->() const  { return dynamic_cast<RTXAcceleratorData*>(_dataPtr.get()); };
    };

};

namespace vrt {
    // passing structure
    class VtRTXAccelerationExtension : public VtDeviceAdvancedAccelerationExtension {
    public:
        friend VtDeviceAdvancedAccelerationExtension;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };


    };
};
