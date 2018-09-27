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
    auto VT_ACCELERATOR_NAME_RTX = VtAcceleratorName(0x00001000u); // planned in 2019

    // passing structure
    class VtRTXAccelerationExtension : public VtDeviceAccelerationExtension {
    public: 
        friend VtDeviceAccelerationExtension;
        VtAcceleratorName _acceleratorName = VT_ACCELERATOR_NAME_RTX;
    };

};


// low-level implementations (WIP)
namespace _vt {


    // planned in 2019
    class RTXAcceleratorSetExtension : public AcceleratorSetExtensionBase, std::enable_shared_from_this<RTXAcceleratorSetExtension> {
    public:
        friend Device;
        friend AcceleratorSetExtensionBase;
        VtAcceleratorName _acceleratorName = VT_ACCELERATOR_NAME_RTX; // identify as RTX 
    };

    // planned in 2019
    class RTXAccelerator : public AdvancedAcceleratorBase, std::enable_shared_from_this<RTXAccelerator> {
    public:
        friend Device;
        friend AdvancedAcceleratorBase;
        VtAcceleratorName _acceleratorName = VT_ACCELERATOR_NAME_RTX; // identify as RTX 

    };
};