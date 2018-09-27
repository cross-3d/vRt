#pragma once
#include "vRt/vRt.h"


// 
namespace _vt {
    class RTXAcceleratorSetExtension;
    class RTXAccelerator;
};


// planned in 2019
namespace vrt {

    // extending to enum type
    auto VT_ACCELERATION_NAME_RTX = VtAccelerationName(0x00001000u); // planned in 2019

    // passing structure
    class VtRTXAccelerationExtension : public VtDeviceAccelerationExtension {
    public: 
        friend VtDeviceAccelerationExtension;

        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };


    };

};


// lower level hard classes (WIP)
#include "vRt/Backland/Definitions/HardClasses.inl"

namespace _vt {


    class RTXAcceleratorSetExtensionData : public AcceleratorSetExtensionDataBase, std::enable_shared_from_this<RTXAcceleratorSetExtensionData> {
    public:
        friend Device;


    };

    // planned in 2019
    class RTXAcceleratorSetExtension : public AcceleratorSetExtensionBase, std::enable_shared_from_this<RTXAcceleratorSetExtension> {
    public:
        friend Device;
        friend AcceleratorSetExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

        // built-in operators for getting inner data 
        auto* operator->()  { return (RTXAcceleratorSetExtensionData*)_dataPtr.get(); };
        auto* operator->() const  { return (RTXAcceleratorSetExtensionData*)_dataPtr.get(); };
    };


    class RTXAcceleratorData : public AdvancedAcceleratorDataBase, std::enable_shared_from_this<RTXAcceleratorData> {
    public:
        friend Device;


    };

    // planned in 2019
    class RTXAccelerator : public AdvancedAcceleratorBase, std::enable_shared_from_this<RTXAccelerator> {
    public:
        friend Device;
        friend AdvancedAcceleratorBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

        // built-in operators for getting inner data 
        auto* operator->()  { return (RTXAcceleratorData*)_dataPtr.get(); };
        auto* operator->() const  { return (RTXAcceleratorData*)_dataPtr.get(); };
    };
};