#pragma once
#include <algorithm>
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // constructor for accelerator set when enabled extension
    VtResult RTXAcceleratorSetExtension::_Construction(std::shared_ptr<AcceleratorSet> accelSet) {
        VkGeometryTrianglesNVX _vertexProxyNVX = vk::GeometryTrianglesNVX{};
        _vertexProxyNVX.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        _vertexProxyNVX.vertexOffset = 0ull;
        _vertexProxyNVX.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNVX.vertexData = VK_NULL_HANDLE;
        _vertexProxyNVX.vertexCount = accelSet->_capacity * 3ull;

        _vertexProxyNVX.indexType = VK_INDEX_TYPE_UINT32;
        _vertexProxyNVX.indexCount = accelSet->_capacity * 3ull;
        _vertexProxyNVX.indexOffset = 0ull;
        _vertexProxyNVX.indexData = VK_NULL_HANDLE;

        VkGeometryDataNVX _vertexDataNVX = vk::GeometryDataNVX{};
        _vertexDataNVX.aabbs = vk::GeometryAABBNVX{};
        _vertexDataNVX.triangles = _vertexProxyNVX;

        VkGeometryNVX _vDataNVX = vk::GeometryNVX{};
        _vDataNVX.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NVX;
        _vDataNVX.geometry = _vertexDataNVX;


         
        // creation of accelerator structure
        VkAccelerationStructureCreateInfoNVX _accelerationCreate = vk::AccelerationStructureCreateInfoNVX{};
        if (accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE) {
            _accelerationCreate.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NVX;
            _accelerationCreate.instanceCount = uint32_t(accelSet->_capacity);
        }
        else {
            _accelerationCreate.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NVX;
            _accelerationCreate.geometryCount = 1u;
            _accelerationCreate.pGeometries = &_vDataNVX;
        };

        const auto buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NVX;
        _accelerationCreate.flags = buildFlags;


        // create acceleration structure (no memory bind)
        vkCreateAccelerationStructureNVX(*accelSet->_device, &_accelerationCreate, nullptr, &_accelStructureNVX);


        // allocate and bind acceleration structure memory 
        { // VMA doesn't have native RTX memory support, so try to allocate manually
            VkAccelerationStructureMemoryRequirementsInfoNVX sMem = vk::AccelerationStructureMemoryRequirementsInfoNVX{};
            sMem.accelerationStructure = _accelStructureNVX;

            // get memory requirements 
            VkMemoryRequirements2 mRequirements = vk::MemoryRequirements2{};
            vkGetAccelerationStructureMemoryRequirementsNVX(*accelSet->_device, &sMem, &mRequirements);

            // allocate by VMA 
            VmaAllocationCreateInfo vmaAlloc = {};
            vmaAlloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaAllocateMemory(accelSet->_device->_allocator, &mRequirements.memoryRequirements, &vmaAlloc, &_vmaAllocation, &_vmaAllocationInfo);

            // bind memory with acceleration structure 
            VkBindAccelerationStructureMemoryInfoNVX mBind = vk::BindAccelerationStructureMemoryInfoNVX{};
            mBind.accelerationStructure = _accelStructureNVX;
            mBind.memory = _vmaAllocationInfo.deviceMemory;
            mBind.memoryOffset = _vmaAllocationInfo.offset;
            vkBindAccelerationStructureMemoryNVX(*accelSet->_device, 1, &mBind);
        };


        // scratch memory allocation
        { // VMA doesn't have native RTX memory support, so try to allocate manually
            VkAccelerationStructureMemoryRequirementsInfoNVX sMem = vk::AccelerationStructureMemoryRequirementsInfoNVX{};
            sMem.accelerationStructure = _accelStructureNVX;

            // get scratch memory requirements
            VkMemoryRequirements2 mRequirements = {};
            vkGetAccelerationStructureScratchMemoryRequirementsNVX(*accelSet->_device, &sMem, &mRequirements);

            // just create scratch memory
            VtDeviceBufferCreateInfo dbs = {};
            dbs.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            dbs.usageFlag |= VK_BUFFER_USAGE_RAYTRACING_BIT_NVX;
            dbs.bufferSize = mRequirements.memoryRequirements.size;
            createDeviceBuffer(accelSet->_device, dbs, _scratchBuffer);
        };

        if (accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE) {
            // create description of structure to bind into descriptor set
            _accelDescriptorNVX = vk::DescriptorAccelerationStructureInfoNVX{};
            _accelDescriptorNVX.accelerationStructureCount = 1;
            _accelDescriptorNVX.pAccelerationStructures = &_accelStructureNVX;

            auto hAccExtension = std::dynamic_pointer_cast<RTXAcceleratorExtension>(accelSet->_device->_hExtensionAccelerator[0]);
            _accelDescriptorSetNVX = vk::Device(VkDevice(*accelSet->_device)).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(accelSet->_device->_descriptorPool).setPSetLayouts((vk::DescriptorSetLayout*)&hAccExtension->_raytracingDescriptorLayout).setDescriptorSetCount(1))[0];

            std::vector<vk::WriteDescriptorSet> writes = { vk::WriteDescriptorSet(_accelDescriptorSetNVX, 0, 0, 1, vk::DescriptorType::eAccelerationStructureNVX).setPNext(&_accelDescriptorNVX).setDstBinding(0u) };
            vk::Device(VkDevice(*accelSet->_device)).updateDescriptorSets(writes, {});
        };


        return VK_SUCCESS;
        //return VK_ERROR_EXTENSION_NOT_PRESENT;
    };


    VtResult RTXAcceleratorExtension::_ConstructAcceleratorSet(std::shared_ptr<AcceleratorSet> accelSet) {
        auto accelSetExt = std::make_shared<RTXAcceleratorSetExtension>();
        accelSet->_hExtension = accelSetExt;
        return accelSetExt->_Construction(accelSet);
    };


};
