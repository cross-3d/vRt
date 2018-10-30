#pragma once
#include <algorithm>
#include "../../Backland/vRt_subimpl.inl"
#include "../RTXClasses.inl"

namespace _vt {

    // constructor for accelerator set when enabled extension
    VtResult RTXAcceleratorSetExtension::_Construction(std::shared_ptr<AcceleratorSet> accelSet) {
        VkGeometryTrianglesNVX _vertexProxyNVX = vk::GeometryTrianglesNVX{};
        _vertexProxyNVX.vertexFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        _vertexProxyNVX.vertexOffset = 0ull;
        _vertexProxyNVX.vertexStride = sizeof(float) * 4ull;
        _vertexProxyNVX.vertexData = nullptr;
        _vertexProxyNVX.vertexCount = accelSet->_capacity;

        VkGeometryDataNVX _vertexDataNVX = vk::GeometryDataNVX{};
        _vertexDataNVX.aabbs = vk::GeometryAABBNVX{};
        _vertexDataNVX.triangles = _vertexProxyNVX;

        VkGeometryNVX _vDataNVX = vk::GeometryNVX{};
        _vDataNVX.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NVX;
        _vDataNVX.geometry = _vertexDataNVX;

        // creation of accelerator structure
        VkAccelerationStructureCreateInfoNVX _accelerationCreate = vk::AccelerationStructureCreateInfoNVX{};
        _accelerationCreate.instanceCount = accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE ? uint32_t(accelSet->_capacity) : 0u;
        _accelerationCreate.type = accelSet->_level == VT_ACCELERATOR_SET_LEVEL_INSTANCE ? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NVX : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NVX;
        _accelerationCreate.geometryCount = accelSet->_level == VT_ACCELERATOR_SET_LEVEL_GEOMETRY ? 1u : 0u;
        _accelerationCreate.pGeometries = accelSet->_level == VT_ACCELERATOR_SET_LEVEL_GEOMETRY ? &_vDataNVX : nullptr;
        _accelerationCreate.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NVX | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NVX;


        // create acceleration structure (no memory bind)
        vkCreateAccelerationStructureNVX(*accelSet->_device, &_accelerationCreate, nullptr, &_accelStructureNVX);

        // create description of structure to bind into descriptor set
        _accelDescriptorNVX = vk::DescriptorAccelerationStructureInfoNVX{};
        _accelDescriptorNVX.accelerationStructureCount = 1;
        _accelDescriptorNVX.pAccelerationStructures = &_accelStructureNVX;


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

            // allocate by VMA 
            VmaAllocationCreateInfo vmaAlloc = {};
            vmaAlloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaAllocateMemory(accelSet->_device->_allocator, &mRequirements.memoryRequirements, &vmaAlloc, &_vmaScratchAllocation, &_vmaScratchAllocationInfo);

            // 
            auto binfo = VkBufferCreateInfo(vk::BufferCreateInfo{});
            binfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            binfo.size = mRequirements.memoryRequirements.size;
            binfo.usage = VK_BUFFER_USAGE_RAYTRACING_BIT_NVX;

            // create buffer with scratch memory
            VkBuffer scratchMemoryBuffer = {}; // TODO: VkBuffer input support
            if (!scratchMemoryBuffer) {
                vkCreateBuffer(*accelSet->_device, &binfo, nullptr, &scratchMemoryBuffer);
            };

            // create buffer region and bind scratch memory
            VtBufferRegionCreateInfo brg = {};
            createBufferRegion(scratchMemoryBuffer, brg, _scratchBuffer, accelSet->_device);
            vmaBindBufferMemory(accelSet->_device->_allocator, _vmaScratchAllocation, scratchMemoryBuffer);
        };


        {
            auto hAccExtension = std::dynamic_pointer_cast<RTXAcceleratorExtension>(accelSet->_device->_hExtensionAccelerator[0]);
            _accelDescriptorSetNVX = vk::Device(VkDevice(*accelSet->_device)).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(accelSet->_device->_descriptorPool).setPSetLayouts((vk::DescriptorSetLayout*)&hAccExtension->_raytracingDescriptorLayout).setDescriptorSetCount(1))[0];

            std::vector<vk::WriteDescriptorSet> writes = { vk::WriteDescriptorSet(_accelDescriptorSetNVX, 0, 0, 1, vk::DescriptorType::eAccelerationStructureNVX).setPNext(&_accelDescriptorNVX) };
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
