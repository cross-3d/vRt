#pragma once

// implementable
//#include "../../vRt_subimpl.inl"
#include "../../Definitions/HardClasses.inl"

// implement of handlers
namespace vrt { // store in official namespace

     VtInstance::operator VkInstance() const {return *_vtHandle;};
     VtPhysicalDevice::operator VkPhysicalDevice() const {return *_vtHandle;};
     VtCommandBuffer::operator VkCommandBuffer() const {return *_vtHandle;};
     VtPipelineLayout::operator VkPipelineLayout() const {return *_vtHandle;};
     VtVertexInputSet::operator VkDescriptorSet() const {return *_vtHandle;};
     VtRayTracingSet::operator VkDescriptorSet() const { return *_vtHandle; };
     VtAcceleratorSet::operator VkDescriptorSet() const { return *_vtHandle; };
     VtVertexAssemblySet::operator VkDescriptorSet() const { return *_vtHandle; };
     VtMaterialSet::operator VkDescriptorSet() const { return *_vtHandle; };

     VtDeviceImage::operator VkImage() const { return *_vtHandle; };
     VtDeviceImage::operator VkImage&() { return *_vtHandle; };
     VtDeviceImage::operator VkImageView() const { return *_vtHandle; };
     VtDeviceImage::operator VkImageView&() { return *_vtHandle; };

     VtDevice::operator VkDevice() const { return *_vtHandle; };
     VtDevice::operator VkPipelineCache() const { return *_vtHandle; };
     VtDevice::operator VkDescriptorPool() const { return *_vtHandle; };

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
     VtDevice::operator VmaAllocator() const { return _vtHandle->_allocator; };
     const void* VtDevice::_getAllocator() const { return &_vtHandle->_allocator; };
#else
     const void* VtDevice::_getAllocator() const { return nullptr; };
#endif

     VtDevice::operator VtHostToDeviceBuffer() const { return VtHostToDeviceBuffer{ _vtHandle->_bufferTraffic->_uploadBuffer }; };
     VtDevice::operator VtDeviceToHostBuffer() const { return VtDeviceToHostBuffer{ _vtHandle->_bufferTraffic->_downloadBuffer }; };

    // getter of descriptor layout from device VtDevice
#ifdef VRT_ENABLE_STRING_VIEW
     VkDescriptorSetLayout VtDevice::getDescriptorLayout(const std::string_view& name) const {
#else
     VkDescriptorSetLayout VtDevice::getDescriptorLayout(const std::string& name) const {
#endif
            return _vtHandle->_descriptorLayoutMap[std::string(name)];
    };

    // templated implementations
    //template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBuffer() const { return *P::_vtHandle; };
    //template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBuffer&() { return *P::_vtHandle; };
    //template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBufferView() const { return *P::_vtHandle; };
    //template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBufferView&() { return *P::_vtHandle; };
};
