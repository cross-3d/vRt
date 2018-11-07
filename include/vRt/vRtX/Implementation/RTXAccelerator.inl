#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    
    static inline auto makePipelineStageInfo(VkDevice device, std::string fpath = "", const char * entry = "main", VkShaderStageFlagBits stage = VK_SHADER_STAGE_RAYGEN_BIT_NV) {
        std::vector<uint32_t> code = readBinary(fpath);

        VkPipelineShaderStageCreateInfo spi = vk::PipelineShaderStageCreateInfo{};
        spi.module = {};
        spi.flags = {};
        createShaderModuleIntrusive(device, code, spi.module);
        spi.pName = entry;
        spi.stage = stage;
        spi.pSpecializationInfo = {};
        return spi;
    };

    static inline VkDeviceSize sMin(VkDeviceSize a, VkDeviceSize b) { return a > b ? b : a; };
    const uint32_t _RTXgroupCount = 3u;


    VtResult RTXAcceleratorExtension::_DoIntersections(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> accel, std::shared_ptr<RayTracingSet> rtset) {
        const auto extendedSet = std::dynamic_pointer_cast<RTXAcceleratorSetExtension>(accel->_hExtension);
        const auto accelertExt = std::dynamic_pointer_cast<RTXAcceleratorExtension>(accel->_device->_hExtensionAccelerator[0]);

        std::vector<uint32_t> _offsets = {};
        //std::vector<vk::DescriptorSet> _tvSets = { rtset->_descriptorSet, extendedSet->_accelDescriptorSetNV, (accel->_vertexAssemblySet)->_descriptorSet };
        std::vector<vk::DescriptorSet> _tvSets = { rtset->_descriptorSet, extendedSet->_accelDescriptorSetNV };
        
        auto cmdBufVk = vk::CommandBuffer(VkCommandBuffer(*cmdBuf));
        //cmdUpdateBuffer(cmdBufVk, VkBuffer(*_sbtBuffer), 0ull, _raytracingProperties.shaderHeaderSize * _RTXgroupCount, &_sbtData); 
        cmdBufVk.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, accelertExt->_intersectionPipelineNV);
        cmdBufVk.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, vk::PipelineLayout(accelertExt->_raytracingPipelineLayout), 0, _tvSets, _offsets);
        cmdBufVk.traceRaysNV(
            vk::Buffer(VkBuffer(*accelertExt->_sbtBuffer)), 0ull,
            vk::Buffer(VkBuffer(*accelertExt->_sbtBuffer)), 2ull * _raytracingProperties.shaderGroupHandleSize, _raytracingProperties.shaderGroupHandleSize,
            vk::Buffer(VkBuffer(*accelertExt->_sbtBuffer)), 1ull * _raytracingProperties.shaderGroupHandleSize, _raytracingProperties.shaderGroupHandleSize,
            {}, 0, 0,
            4608u, 1u, 1u);
        cmdRaytracingBarrierNV(cmdBufVk);


        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };


    VtResult RTXAcceleratorExtension::_BuildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> accelSet, VtAcceleratorBuildInfo buildInfo) {
        // if has valid vertex assembly
        if (!(accelSet->_vertexAssemblySet && accelSet->_vertexAssemblySet->_hExtension && accelSet->_vertexAssemblySet->_hExtension->_AccelerationName() == VT_ACCELERATION_NAME_RTX)) return VK_ERROR_EXTENSION_NOT_PRESENT;

        auto vertexAssemblyExtension = std::dynamic_pointer_cast<RTXVertexAssemblyExtension>(accelSet->_vertexAssemblySet->_hExtension);

        auto& _trianglesProxy = vertexAssemblyExtension->_vDataNV.geometry.triangles;
        _trianglesProxy.vertexCount = accelSet->_vertexAssemblySet->_calculatedPrimitiveCount * 3ull;
        _trianglesProxy.vertexOffset = accelSet->_vertexAssemblySet->_verticeBufferCached->_offset();
        _trianglesProxy.vertexData = VkBuffer(*accelSet->_vertexAssemblySet->_verticeBufferCached);

        //vertexAssemblyExtension->_vertexProxyNV.indexType = VK_INDEX_TYPE_UINT32;
        _trianglesProxy.indexCount = accelSet->_vertexAssemblySet->_calculatedPrimitiveCount * 3ull;
        _trianglesProxy.indexOffset = accelSet->_vertexAssemblySet->_indexBuffer->_offset();
        _trianglesProxy.indexOffset += (buildInfo.elementOffset + accelSet->_elementsOffset)*(3ull*sizeof(uint32_t));
        _trianglesProxy.indexData = VK_NULL_HANDLE;//VkBuffer(*accelSet->_vertexAssemblySet->_indexBuffer);
        

        const auto vsize = accelSet->_vertexAssemblySet && accelSet->_level == VT_ACCELERATOR_SET_LEVEL_GEOMETRY ? VkDeviceSize(accelSet->_vertexAssemblySet->_calculatedPrimitiveCount) : VK_WHOLE_SIZE;
        const auto dsize = uint32_t(sMin((accelSet->_elementsCount != -1 && accelSet->_elementsCount >= 0) ? VkDeviceSize(accelSet->_elementsCount) : VkDeviceSize(vsize), sMin(buildInfo.elementSize, accelSet->_capacity)));

        //const auto buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
        const auto buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
        const auto extendedSet = std::dynamic_pointer_cast<RTXAcceleratorSetExtension>(accelSet->_hExtension);



        auto cmdBufVk = vk::CommandBuffer(VkCommandBuffer(*cmdBuf));
        if (accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE) {
            extendedSet->_accelInfoNV.instanceCount = dsize;
            vkCmdBuildAccelerationStructureNV(cmdBufVk, &extendedSet->_accelInfoNV, *accelSet->_bvhInstancedBuffer, accelSet->_bvhInstancedBuffer->_offset(), extendedSet->_WasBuild, extendedSet->_accelStructureNV, VK_NULL_HANDLE, *extendedSet->_scratchBuffer, extendedSet->_scratchBuffer->_offset());
        }
        else {
            extendedSet->_accelInfoNV.pGeometries = &vertexAssemblyExtension->_vDataNV;
            vkCmdBuildAccelerationStructureNV(cmdBufVk, &extendedSet->_accelInfoNV, VK_NULL_HANDLE, 0ull, extendedSet->_WasBuild, extendedSet->_accelStructureNV, VK_NULL_HANDLE, *extendedSet->_scratchBuffer, extendedSet->_scratchBuffer->_offset());
        };
        //extendedSet->_WasBuild = true; // force re-build structure
        cmdRaytracingBarrierNV(cmdBufVk);
        

        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

    VtResult RTXAcceleratorExtension::_Init(std::shared_ptr<Device> device, const VtDeviceAdvancedAccelerationExtension * extensionBasedInfo) {
        const auto * extensionInfo = (VtRTXAcceleratorExtension*)(extensionBasedInfo);
        _raytracingProperties = device->_features->_rayTracingNV; // planned to merge here

        // create SBT buffer
        VtDeviceBufferCreateInfo dbi = {};
        dbi.bufferSize = _raytracingProperties.shaderGroupHandleSize * _RTXgroupCount;
        dbi.usageFlag = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        createHostToDeviceBuffer(device, dbi, _sbtBuffer);

        //
        auto pbindings = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind | vk::DescriptorBindingFlagBitsEXT::eVariableDescriptorCount | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;
        auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(&pbindings);
        auto vkpi = vk::DescriptorSetLayoutCreateInfo().setPNext(&vkfl);

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0u, vk::DescriptorType::eAccelerationStructureNV, 1, vk::ShaderStageFlagBits::eRaygenNV), // rays
            };
            _raytracingDescriptorLayout = vk::Device(VkDevice(*device)).createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(device->_descriptorLayoutMap["rayTracing"]),
                _raytracingDescriptorLayout,
                //vk::DescriptorSetLayout(device->_descriptorLayoutMap["vertexData"]),
            };

            std::vector<vk::PushConstantRange> constRanges = {};
            _raytracingPipelineLayout = vk::Device(VkDevice(*device)).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
        };

        {
            const auto vendorName = device->_vendorName;

            std::vector<VkRayTracingShaderGroupCreateInfoNV> groups = {
                vk::RayTracingShaderGroupCreateInfoNV().setType(vk::RayTracingShaderGroupTypeNV::eGeneral).setGeneralShader(0),
                vk::RayTracingShaderGroupCreateInfoNV().setType(vk::RayTracingShaderGroupTypeNV::eTrianglesHitGroup).setClosestHitShader(1).setAnyHitShader(2),
                vk::RayTracingShaderGroupCreateInfoNV().setType(vk::RayTracingShaderGroupTypeNV::eGeneral).setGeneralShader(3)
            };

            std::vector<VkPipelineShaderStageCreateInfo> stages = {
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rgen", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_RAYGEN_BIT_NV),
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rchit", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV),
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rahit", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_ANY_HIT_BIT_NV),
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rmiss", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_MISS_BIT_NV),
            };

            VkRayTracingPipelineCreateInfoNV rayPipelineInfo = vk::RayTracingPipelineCreateInfoNV{};
            rayPipelineInfo.stageCount = (uint32_t)stages.size();
            rayPipelineInfo.groupCount = (uint32_t)groups.size();
            rayPipelineInfo.pStages = stages.data();
            rayPipelineInfo.pGroups = groups.data();
            rayPipelineInfo.layout = _raytracingPipelineLayout;
            rayPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            rayPipelineInfo.basePipelineIndex = 0;
            rayPipelineInfo.maxRecursionDepth = 1;
            
            vkCreateRayTracingPipelinesNV(VkDevice(*device), {}, 1, &rayPipelineInfo, nullptr, &_intersectionPipelineNV);
            vkGetRayTracingShaderGroupHandlesNV(VkDevice(*device), _intersectionPipelineNV, 0, _RTXgroupCount, size_t(_raytracingProperties.shaderGroupHandleSize * _RTXgroupCount), _sbtBuffer->_hostMapped());
        };

        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };


    // unused 
    //VtResult RTXAcceleratorExtension::_Criteria(std::shared_ptr<DeviceFeatures> supportedFeatures) {
    //    return VK_ERROR_EXTENSION_NOT_PRESENT;
    //};


};
