#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    VtResult RTXAcceleratorExtension::_DoIntersections(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, std::shared_ptr<RayTracingSet> rayTracingSet) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };
    VtResult RTXAcceleratorExtension::_BuildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, VtAcceleratorBuildInfo buildInfo = {}) {
        // if has valid vertex assembly
        if (acceleratorSet->_vertexAssemblySet && acceleratorSet->_vertexAssemblySet->_hExtension && acceleratorSet->_vertexAssemblySet->_hExtension->_AccelerationName == VT_ACCELERATION_NAME_RTX) {
            auto vertexAssemblyExtension = std::dynamic_pointer_cast<RTXVertexAssemblyExtension>(acceleratorSet->_vertexAssemblySet->_hExtension);
            vertexAssemblyExtension->_vertexProxyNVX.vertexCount = acceleratorSet->_vertexAssemblySet->_calculatedPrimitiveCount;
        };

        // to be continue...

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };
    VtResult RTXAcceleratorExtension::_Init(std::shared_ptr<Device> device, VtDeviceAdvancedAccelerationExtension extensionInfo) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };
    VtResult RTXAcceleratorExtension::_Criteria(std::shared_ptr<DeviceFeatures> supportedFeatures) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

};
