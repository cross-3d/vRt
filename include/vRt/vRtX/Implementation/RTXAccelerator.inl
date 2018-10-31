#pragma once
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // create shader module
    /*
    static inline auto makePipelineStageInfo(VkDevice device, const std::vector<uint32_t>& code, const char * entry = "main", VkShaderStageFlagBits stage = VK_SHADER_STAGE_RAYGEN_BIT_NVX) {
        VkPipelineShaderStageCreateInfo spi = vk::PipelineShaderStageCreateInfo{};
        spi.module = {};
        spi.flags = {};
        createShaderModuleIntrusive(device, code, spi.module);
        spi.pName = entry;
        spi.stage = stage;
        spi.pSpecializationInfo = {};
        return spi;
    };*/

    static inline auto makePipelineStageInfo(VkDevice device, std::string fpath = "", const char * entry = "main", VkShaderStageFlagBits stage = VK_SHADER_STAGE_RAYGEN_BIT_NVX) {
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
        //std::vector<vk::DescriptorSet> _tvSets = { rtset->_descriptorSet, extendedSet->_accelDescriptorSetNVX, (accel->_vertexAssemblySet)->_descriptorSet };
        std::vector<vk::DescriptorSet> _tvSets = { rtset->_descriptorSet, extendedSet->_accelDescriptorSetNVX };
        
        auto cmdBufVk = vk::CommandBuffer(VkCommandBuffer(*cmdBuf));
        cmdRaytracingBarrierNVX(cmdBufVk);
        //cmdUpdateBuffer(cmdBufVk, VkBuffer(*_sbtBuffer), 0ull, _raytracingProperties.shaderHeaderSize * _RTXgroupCount, &_sbtData);
        cmdRaytracingBarrierNVX(cmdBufVk);
        cmdBufVk.bindPipeline(vk::PipelineBindPoint::eRaytracingNVX, accelertExt->_intersectionPipelineNVX);
        cmdBufVk.bindDescriptorSets(vk::PipelineBindPoint::eRaytracingNVX, vk::PipelineLayout(accelertExt->_raytracingPipelineLayout), 0, _tvSets, _offsets);
        cmdBufVk.traceRaysNVX(
            vk::Buffer(VkBuffer(*accelertExt->_sbtBuffer)), 0ull,
            vk::Buffer(VkBuffer(*accelertExt->_sbtBuffer)), 2ull * _raytracingProperties.shaderHeaderSize, _raytracingProperties.shaderHeaderSize,
            vk::Buffer(VkBuffer(*accelertExt->_sbtBuffer)), 1ull * _raytracingProperties.shaderHeaderSize, _raytracingProperties.shaderHeaderSize,
            4608u, 1u);
        cmdRaytracingBarrierNVX(cmdBufVk);


        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };


    VtResult RTXAcceleratorExtension::_BuildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> accelSet, VtAcceleratorBuildInfo buildInfo) {
        // if has valid vertex assembly
        if (!(accelSet->_vertexAssemblySet && accelSet->_vertexAssemblySet->_hExtension && accelSet->_vertexAssemblySet->_hExtension->_AccelerationName() == VT_ACCELERATION_NAME_RTX)) return VK_ERROR_EXTENSION_NOT_PRESENT;

        auto vertexAssemblyExtension = std::dynamic_pointer_cast<RTXVertexAssemblyExtension>(accelSet->_vertexAssemblySet->_hExtension);

        auto& _trianglesProxy = vertexAssemblyExtension->_vDataNVX.geometry.triangles;
        _trianglesProxy.vertexCount = accelSet->_vertexAssemblySet->_calculatedPrimitiveCount * 3ull;
        _trianglesProxy.vertexOffset = accelSet->_vertexAssemblySet->_verticeBufferCached->_offset();
        _trianglesProxy.vertexData = VkBuffer(*accelSet->_vertexAssemblySet->_verticeBufferCached);

        //vertexAssemblyExtension->_vertexProxyNVX.indexType = VK_INDEX_TYPE_UINT32;
        _trianglesProxy.indexCount = accelSet->_vertexAssemblySet->_calculatedPrimitiveCount * 3ull;
        _trianglesProxy.indexOffset = accelSet->_vertexAssemblySet->_indexBuffer->_offset();
        _trianglesProxy.indexData = VK_NULL_HANDLE;//VkBuffer(*accelSet->_vertexAssemblySet->_indexBuffer);
        

        const auto vsize = accelSet->_vertexAssemblySet && accelSet->_level == VT_ACCELERATOR_SET_LEVEL_GEOMETRY ? VkDeviceSize(accelSet->_vertexAssemblySet->_calculatedPrimitiveCount) : VK_WHOLE_SIZE;
        const auto dsize = uint32_t(sMin((accelSet->_elementsCount != -1 && accelSet->_elementsCount >= 0) ? VkDeviceSize(accelSet->_elementsCount) : VkDeviceSize(vsize), sMin(buildInfo.elementSize, accelSet->_capacity)));

        const auto buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NVX;
        const auto extendedSet = std::dynamic_pointer_cast<RTXAcceleratorSetExtension>(accelSet->_hExtension);



        auto cmdBufVk = vk::CommandBuffer(VkCommandBuffer(*cmdBuf));
        cmdRaytracingBarrierNVX(cmdBufVk);

        if (accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE) {
            vkCmdBuildAccelerationStructureNVX(cmdBufVk,
                VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NVX,
                dsize, *accelSet->_bvhInstancedBuffer, accelSet->_bvhInstancedBuffer->_offset(), 
                0, nullptr, buildFlags, VK_FALSE,
                extendedSet->_accelStructureNVX, VK_NULL_HANDLE,
                *extendedSet->_scratchBuffer, extendedSet->_scratchBuffer->_offset()
            );
        }
        else {
            vkCmdBuildAccelerationStructureNVX(cmdBufVk,
                VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NVX,
                0, nullptr, 0,
                1, &vertexAssemblyExtension->_vDataNVX, buildFlags, VK_FALSE,
                extendedSet->_accelStructureNVX, VK_NULL_HANDLE,
                *extendedSet->_scratchBuffer, extendedSet->_scratchBuffer->_offset()
            );
        };

        cmdRaytracingBarrierNVX(cmdBufVk);
        

        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

    VtResult RTXAcceleratorExtension::_Init(std::shared_ptr<Device> device, const VtDeviceAdvancedAccelerationExtension * extensionBasedInfo) {
        const auto * extensionInfo = (VtRTXAcceleratorExtension*)(extensionBasedInfo);
        _raytracingProperties = device->_features->_raytracingNVX; // planned to merge here

        // create SBT buffer
        VtDeviceBufferCreateInfo dbi = {};
        dbi.bufferSize = _raytracingProperties.shaderHeaderSize * _RTXgroupCount;
        dbi.usageFlag = VK_BUFFER_USAGE_RAYTRACING_BIT_NVX;
        createHostToDeviceBuffer(device, dbi, _sbtBuffer);

        //
        auto pbindings = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind | vk::DescriptorBindingFlagBitsEXT::eVariableDescriptorCount | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;
        auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(&pbindings);
        auto vkpi = vk::DescriptorSetLayoutCreateInfo().setPNext(&vkfl);

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0u, vk::DescriptorType::eAccelerationStructureNVX, 1, vk::ShaderStageFlagBits::eRaygenNVX), // rays
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
            //const uint32_t groupNumbers[] = { 0, 1, 2 };
            const uint32_t groupNumbers[] = { 0, 1, 1, 2 };

            std::vector<VkPipelineShaderStageCreateInfo> stages = {
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rgen", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_RAYGEN_BIT_NVX),
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rchit", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_CLOSEST_HIT_BIT_NVX),
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rahit", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_ANY_HIT_BIT_NVX),
                makePipelineStageInfo(VkDevice(*device), getCorrectPath("accelNVX/traverse.rmiss", vendorName, device->_shadersPath), "main", VK_SHADER_STAGE_MISS_BIT_NVX),
            };

            VkRaytracingPipelineCreateInfoNVX rayPipelineInfo = vk::RaytracingPipelineCreateInfoNVX{};
            rayPipelineInfo.stageCount = (uint32_t)stages.size();
            rayPipelineInfo.pStages = stages.data();
            rayPipelineInfo.pGroupNumbers = &groupNumbers[0];
            rayPipelineInfo.layout = _raytracingPipelineLayout;
            rayPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            rayPipelineInfo.basePipelineIndex = 0;
            rayPipelineInfo.maxRecursionDepth = 1;
            vkCreateRaytracingPipelinesNVX(VkDevice(*device), device->_pipelineCache, 1, &rayPipelineInfo, nullptr, &_intersectionPipelineNVX);
            vkGetRaytracingShaderHandlesNVX(VkDevice(*device), _intersectionPipelineNVX, 0, _RTXgroupCount, size_t(_raytracingProperties.shaderHeaderSize * _RTXgroupCount), _sbtBuffer->_hostMapped());
        };

        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };


    // unused 
    //VtResult RTXAcceleratorExtension::_Criteria(std::shared_ptr<DeviceFeatures> supportedFeatures) {
    //    return VK_ERROR_EXTENSION_NOT_PRESENT;
    //};


};
