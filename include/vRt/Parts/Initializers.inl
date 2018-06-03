#pragma once
#include "Vulkan.inl"
#include "HardClasses.inl"

// C++ internal initializers for hard classes (interface)
namespace _vt { // store in undercover namespace

    inline std::shared_ptr<PhysicalDevice> makePhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical);
    inline std::shared_ptr<Device> createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VtDeviceCreateInfo vdvi);
    inline std::shared_ptr<DeviceBuffer> createDeviceBuffer(std::shared_ptr<Device> device, VkBufferUsageFlagBits usageFlag, uint32_t familyIndex, VkDeviceSize bufferSize = sizeof(uint32_t));

};
