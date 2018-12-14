#pragma once

#include "RTXClassesDef.inl"
#include "../Backland/Definitions/HardClasses.inl"

namespace _vt {

    // 
    class RTXAcceleratorSetExtension : public AcceleratorSetExtensionBase, std::enable_shared_from_this<RTXAcceleratorSetExtension> {
    public:
        friend Device;
        friend AcceleratorSetExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };
        virtual VtResult _Construction(const std::shared_ptr<AcceleratorSet>& accelSet = {}) override;

        // acceleration structure
        VkAccelerationStructureNV _accelStructureNV = {};
        VkAccelerationStructureInfoNV _accelInfoNV = {};

        VmaAllocation _vmaAllocation = {}, _vmaScratchAllocation = {};
        VmaAllocationInfo _vmaAllocationInfo = {}, _vmaScratchAllocationInfo = {};
        VkWriteDescriptorSetAccelerationStructureNV _accelDescriptorNV = {};
        VkDescriptorSet _accelDescriptorSetNV = {}; // additional descriptor set
        


        // scratch buffers
        bool _WasBuild = false;
          std::shared_ptr<DeviceBuffer> _scratchBuffer = {}; // 
        //std::shared_ptr<BufferRegion> _scratchRegion = {}; // 

        operator VkAccelerationStructureNV() const { return _accelStructureNV; };
        operator VkAccelerationStructureNV&() { return _accelStructureNV; };
    };

    // 
    struct RTXShaderBindingTable { uint32_t data[4] = { 0u,0u,0u,0u }; };

    // 
    class RTXAcceleratorExtension : public AcceleratorExtensionBase, std::enable_shared_from_this<RTXAcceleratorExtension> {
    public:
        friend Device;
        friend AcceleratorExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

        // 
        virtual VtResult _DoIntersections(const std::shared_ptr<CommandBuffer>& cmdBuf, const std::shared_ptr<AcceleratorSet>& acceleratorSet, const std::shared_ptr<RayTracingSet>& rayTracingSet) override;
        virtual VtResult _BuildAccelerator(const std::shared_ptr<CommandBuffer>& cmdBuf, const std::shared_ptr<AcceleratorSet>& acceleratorSet, const VtAcceleratorBuildInfo& buildInfo) override;
        virtual VtResult _Init(const std::shared_ptr<Device>& device, const VtDeviceAdvancedAccelerationExtension * extensionBasedInfo) override;
        //virtual VtResult _Criteria(const std::shared_ptr<DeviceFeatures> supportedFeatures) override;
        virtual VtResult _ConstructAcceleratorSet(const std::shared_ptr<AcceleratorSet>& accelSet = {}) override;
        virtual VtResult _ConstructVertexAssembly(const std::shared_ptr<VertexAssemblySet>& assemblySet = {}) override;

        // 
        VkDescriptorSetLayout _raytracingDescriptorLayout = {};
        VkPipelineLayout _raytracingPipelineLayout = {};
        VkPipeline _intersectionPipelineNV = {}; // native RTX intersection system 
        VkPhysicalDeviceRayTracingPropertiesNV _rayTracingNV = {};
        //std::shared_ptr<DeviceBuffer> _sbtBuffer = {};

        RTXShaderBindingTable _sbtDebugData[4] = {};
        std::shared_ptr<HostToDeviceBuffer> _sbtBuffer = {};
    };

    // 
    class RTXVertexAssemblyExtension : public VertexAssemblyExtensionBase, std::enable_shared_from_this<RTXVertexAssemblyExtension> {
    public:
        friend Device;
        friend VertexAssemblyExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };
        virtual VtResult _Construction(const std::shared_ptr<VertexAssemblySet>& _assemblySet = {}) override;

        // 
        VkGeometryNV _vDataNV = {};
        operator VkGeometryNV() const { return _vDataNV; };
        operator VkGeometryNV&() { return _vDataNV; };
    };

};
