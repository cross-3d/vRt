#pragma once
#include <algorithm>
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // constructor for accelerator set when enabled extension
    VtResult RTXAcceleratorSetExtension::_Construction(std::shared_ptr<AcceleratorSet> accelSet) {
        VtResult rtxResult = VK_ERROR_EXTENSION_NOT_PRESENT;

        VkGeometryTrianglesNV _vertexProxyNV = vk::GeometryTrianglesNV{};
        _vertexProxyNV.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNV.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        _vertexProxyNV.vertexCount = accelSet->_capacity * 3ull;

        // still have some dumbs
        _vertexProxyNV.indexType = VK_INDEX_TYPE_NONE_NV; // support was broken
        _vertexProxyNV.indexCount = _vertexProxyNV.vertexCount; // anyways forced requirements

        // enable index buffer
        //_vertexProxyNV.indexType = VK_INDEX_TYPE_UINT32;


        VkGeometryDataNV _vertexDataNV = vk::GeometryDataNV{};
        _vertexDataNV.aabbs = vk::GeometryAABBNV{};
        _vertexDataNV.triangles = _vertexProxyNV;

        VkGeometryNV _vDataNV = vk::GeometryNV{};
        _vDataNV.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        _vDataNV.geometry = _vertexDataNV;
        _vDataNV.flags = VK_GEOMETRY_OPAQUE_BIT_NV | VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV;

        // creation of accelerator structure
        VkAccelerationStructureCreateInfoNV _accelerationCreate = vk::AccelerationStructureCreateInfoNV{};
        
        _accelInfoNV = vk::AccelerationStructureInfoNV{};
        if (accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE) {
            _accelInfoNV.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
            _accelInfoNV.instanceCount = uint32_t(accelSet->_capacity);
        }
        else {
            _accelInfoNV.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
            _accelInfoNV.geometryCount = 1u;
            _accelInfoNV.pGeometries = &_vDataNV;
        };

        //const auto buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
        //const auto buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
        _accelInfoNV.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;


        // create acceleration structure (no memory bind)
        _accelerationCreate.info = _accelInfoNV;
        rtxResult = vkCreateAccelerationStructureNV(*accelSet->_device, &_accelerationCreate, nullptr, &_accelStructureNV), _WasBuild = false;

        // reset create structures
        _accelInfoNV.pGeometries = nullptr;
        _vertexProxyNV.indexCount = 0;


        // allocate and bind acceleration structure memory 
        if (rtxResult == VK_SUCCESS) { // VMA doesn't have native RTX memory support, so try to allocate manually
            VkAccelerationStructureMemoryRequirementsInfoNV sMem = vk::AccelerationStructureMemoryRequirementsInfoNV{};
            sMem.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
            sMem.accelerationStructure = _accelStructureNV;

            // get memory requirements 
            VkMemoryRequirements2 mRequirements = vk::MemoryRequirements2{};
            vkGetAccelerationStructureMemoryRequirementsNV(*accelSet->_device, &sMem, &mRequirements);

            // 
            VkBindAccelerationStructureMemoryInfoNV mBind = vk::BindAccelerationStructureMemoryInfoNV{};
            mBind.accelerationStructure = _accelStructureNV;

            if (mRequirements.memoryRequirements.size > accelSet->_bvhBoxBuffer->_size() || !accelSet->_bvhBoxBuffer->_boundBuffer.lock()) {
                // allocate by VMA 
                VmaAllocationCreateInfo vmaAlloc = {};
                vmaAlloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                vmaAllocateMemory(accelSet->_device->_allocator, &mRequirements.memoryRequirements, &vmaAlloc, &_vmaAllocation, &_vmaAllocationInfo);

                // bind memory with acceleration structure 
                mBind.memory = _vmaAllocationInfo.deviceMemory;
                mBind.memoryOffset = _vmaAllocationInfo.offset;
            }
            else {
                // bind memory with acceleration structure
                auto cbuffer = accelSet->_bvhBoxBuffer->_boundBuffer.lock();
                mBind.memory = cbuffer->_allocationInfo.deviceMemory;
                mBind.memoryOffset = cbuffer->_allocationInfo.offset + accelSet->_bvhBoxBuffer->_offset();
            };

            // bind memory 
            rtxResult = vkBindAccelerationStructureMemoryNV(*accelSet->_device, 1, &mBind);
        };


        // scratch memory allocation
        if (rtxResult == VK_SUCCESS) { // VMA doesn't have native RTX memory support, so try to allocate manually
            VkAccelerationStructureMemoryRequirementsInfoNV sMem = vk::AccelerationStructureMemoryRequirementsInfoNV{};
            sMem.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
            sMem.accelerationStructure = _accelStructureNV;

            // get scratch memory requirements
            VkMemoryRequirements2 mRequirements = {};
            vkGetAccelerationStructureMemoryRequirementsNV(*accelSet->_device, &sMem, &mRequirements);
            //vkGetAccelerationStructureScratchMemoryRequirementsNV(*accelSet->_device, &sMem, &mRequirements);

            // just create scratch memory
            VtDeviceBufferCreateInfo dbs = {};
            dbs.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
            dbs.bufferSize = mRequirements.memoryRequirements.size;
            createDeviceBuffer(accelSet->_device, dbs, _scratchBuffer);
        };

        // create RTX descriptor set
        if (rtxResult == VK_SUCCESS && accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE) {
            // create description of structure to bind into descriptor set
            _accelDescriptorNV = vk::WriteDescriptorSetAccelerationStructureNV{};
            _accelDescriptorNV.accelerationStructureCount = 1;
            _accelDescriptorNV.pAccelerationStructures = &_accelStructureNV;

            auto hAccExtension = std::dynamic_pointer_cast<RTXAcceleratorExtension>(accelSet->_device->_hExtensionAccelerator[0]);
            _accelDescriptorSetNV = vk::Device(VkDevice(*accelSet->_device)).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(accelSet->_device->_descriptorPool).setPSetLayouts((vk::DescriptorSetLayout*)&hAccExtension->_raytracingDescriptorLayout).setDescriptorSetCount(1))[0];

            std::vector<vk::WriteDescriptorSet> writes = { vk::WriteDescriptorSet(_accelDescriptorSetNV, 0u, 0, 1, vk::DescriptorType::eAccelerationStructureNV).setPNext(&_accelDescriptorNV) };
            vk::Device(VkDevice(*accelSet->_device)).updateDescriptorSets(writes, {});
        };

        return rtxResult;
    };


    VtResult RTXAcceleratorExtension::_ConstructAcceleratorSet(std::shared_ptr<AcceleratorSet> accelSet) {
        auto accelSetExt = std::make_shared<RTXAcceleratorSetExtension>();
        accelSet->_hExtension = accelSetExt;
        return accelSetExt->_Construction(accelSet);
    };


};
