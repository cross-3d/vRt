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
        virtual VtResult _Construction(std::shared_ptr<AcceleratorSet> accelSet = {}) override;

        // acceleration structure
        VkAccelerationStructureNVX _accelStructureNVX = {};
        VmaAllocation _vmaAllocation = {}, _vmaScratchAllocation = {};
        VmaAllocationInfo _vmaAllocationInfo = {}, _vmaScratchAllocationInfo = {};
        VkDescriptorAccelerationStructureInfoNVX _accelDescriptorNVX = {};
        VkDescriptorSet _accelDescriptorSetNVX = {}; // additional descriptor set

        // scratch buffers
        std::shared_ptr<BufferRegion> _scratchBuffer = {}; // 
    };

    // 
    class RTXAcceleratorExtension : public AcceleratorExtensionBase, std::enable_shared_from_this<RTXAcceleratorExtension> {
    public:
        friend Device;
        friend AcceleratorExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };

        // 
        virtual VtResult _DoIntersections(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, std::shared_ptr<RayTracingSet> rayTracingSet) override;
        virtual VtResult _BuildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, VtAcceleratorBuildInfo buildInfo = {}) override;
        virtual VtResult _Init(std::shared_ptr<Device> device, const VtDeviceAdvancedAccelerationExtension * extensionBasedInfo) override;
        //virtual VtResult _Criteria(std::shared_ptr<DeviceFeatures> supportedFeatures) override;
        virtual VtResult _ConstructAcceleratorSet(std::shared_ptr<AcceleratorSet> accelSet = {}) override;
        virtual VtResult _ConstructVertexAssembly(std::shared_ptr<VertexAssemblySet> assemblySet = {}) override;

        //  
        VkDescriptorSetLayout _raytracingDescriptorLayout = {};
        VkPipelineLayout _raytracingPipelineLayout = {};
        VkPipeline _intersectionPipelineNVX = {}; // native RTX intersection system 
        VkPhysicalDeviceRaytracingPropertiesNVX _raytracingProperties = {};
        std::shared_ptr<HostToDeviceBuffer> _sbtBuffer = {};

    };

    // 
    class RTXVertexAssemblyExtension : public VertexAssemblyExtensionBase, std::enable_shared_from_this<RTXVertexAssemblyExtension> {
    public:
        friend Device;
        friend VertexAssemblyExtensionBase;
        virtual VtAccelerationName _AccelerationName() const override { return VT_ACCELERATION_NAME_RTX; };
        virtual VtResult _Construction(std::shared_ptr<VertexAssemblySet> _assemblySet = {}) override;

        // 
        VkGeometryTrianglesNVX _vertexProxyNVX = {};
    };

};
