#pragma once
#include "Vulkan.inl"
#include "HardClasses.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace

    std::shared_ptr<PhysicalDevice> makePhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical){
        auto vtPhysical = std::make_shared<PhysicalDevice>();
        vtPhysical->_physicalDevice = physical;
        return vtPhysical;
    };

    std::shared_ptr<Device> createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VtDeviceCreateInfo vdvi){
        auto vtDevice = std::make_shared<Device>();
        vtDevice->_physicalDevice = physicalDevice;

        VkDeviceCreateInfo dvi;
        dvi.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dvi.queueCreateInfoCount = vdvi.queueCreateInfoCount;
        dvi.pQueueCreateInfos = vdvi.pQueueCreateInfos;
        dvi.enabledLayerCount = vdvi.enabledLayerCount;
        dvi.ppEnabledLayerNames = vdvi.ppEnabledLayerNames;
        dvi.enabledExtensionCount = vdvi.enabledExtensionCount;
        dvi.ppEnabledExtensionNames = vdvi.ppEnabledExtensionNames;
        dvi.pEnabledFeatures = vdvi.pEnabledFeatures;
        vkCreateDevice(*vtDevice->_physicalDevice, &dvi, nullptr, &vtDevice->_device);

        VmaAllocatorCreateInfo allocatorInfo;
        allocatorInfo.physicalDevice = *vtDevice->_physicalDevice;
        allocatorInfo.device = vtDevice->_device;
        allocatorInfo.preferredLargeHeapBlockSize = 16384; // 16kb
        allocatorInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vmaCreateAllocator(&allocatorInfo, &vtDevice->_allocator);

        return vtDevice;
    };
    
    std::shared_ptr<DeviceBuffer> createDeviceBuffer(std::shared_ptr<Device> device, VkBufferUsageFlagBits usageFlag, uint32_t familyIndex, VkDeviceSize bufferSize = sizeof(uint32_t)){
        auto vtDeviceBuffer = std::make_shared<DeviceBuffer>();
        
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        auto binfo = VkBufferCreateInfo(VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, bufferSize, usageBits, VkSharingMode::eExclusive, 1, &familyIndex);
        vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo);

        return vtDeviceBuffer;
    };

};