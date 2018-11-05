#pragma once
#include "RTXHandlersDef.inl"


namespace vrt {

    // extension passport for RTX device 
    class VtRTXAcceleratorExtension : VtDeviceAdvancedAccelerationExtension {
    protected: // API in-runtime implementation (dynamic polymorphism)
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; }; // in-runtime return acceleration extension name
        virtual VtResult _Criteria(std::shared_ptr<_vt::DeviceFeatures> lwFeatures) const override;
        virtual VtResult _Initialization(std::shared_ptr<_vt::Device> lwDevice, std::shared_ptr<_vt::AcceleratorExtensionBase>& _hExtensionAccelerator) const override;

    public:

    };



    // simpler handle type 
    using VtHandleRTX = uint64_t;

    // NVidia Ray Tracing Instance Structure (we doesn't allow use "Vk" prefix for any custom definition, so named as "Vt")
    // Also used bit custom constructions, any holywars in issue trackers (such as prefix, suffixes) may cause "Intruder" status with users black-listing 
    // Yes, we can review proposals, but any wars or enforcements is inacceptable! 
    
#pragma pack(push, 1)
    struct VtRTXInstance {
        //float transform[12] = {1.f, 0.f, 0.f, 0.f,  0.f, 1.f, 0.f, 0.f,  0.f, 0.f, 1.f, 0.f };
        VtMat3x4 transform = IdentifyMat3x4; // due this structure located in vRt namespace, prefer use VtMat3x4 type instead regular float[12]
        uint24_t instanceId = 0u; uint8_t mask = 0xFFu;
        uint24_t instanceOffset = 0u; uint8_t flags = 0u;
        VtHandleRTX accelerationStructureHandle = 0ull;
    };
#pragma pack(pop)

    // required for RTX top level support 
    VtResult vtGetAcceleratorHandleNV(VtAcceleratorSet accSet, VtHandleRTX * acceleratorHandleNVX);

};
