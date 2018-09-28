#pragma once

// implementable
#include "../../Parts/HandlersDef.inl" // no, shouldn't 
#include "HardClassesDef.inl" // no, shouldn't 

// implement of handlers
namespace vrt { // store in official namespace

    // unified template for making regular classes
    template<class T>
    class VtHandle {
    protected:
        std::shared_ptr<T> _vtHandle = nullptr;
    public:
        VtHandle(const std::shared_ptr<T>& _H = nullptr) { _vtHandle = _H; }; // constructor
        auto* operator->() { return _vtHandle.get(); };
        auto* operator->() const { return _vtHandle.get(); };
        operator T() const { return *_vtHandle; };
        operator bool() const { return !!_vtHandle; };
        operator std::shared_ptr<T>() const { return _vtHandle; };
        operator std::shared_ptr<T>&() { return _vtHandle; };
        operator std::weak_ptr<T>&&() const { return _vtHandle; };
        //operator T&() { return *_vtHandle; };
    };



    class VtPipeline : public VtHandle<_vt::Pipeline> {};
    class VtAcceleratorHLBVH2 : public VtHandle<_vt::AcceleratorHLBVH2> {};
    class VtVertexAssemblyPipeline : public VtHandle<_vt::VertexAssemblyPipeline> {};

    class VtInstance : public VtHandle<_vt::Instance> {
    public:
        operator VkInstance() const;
    };

    class VtPhysicalDevice : public VtHandle<_vt::PhysicalDevice> {
    public:
        operator VkPhysicalDevice() const;
    };

    class VtCommandBuffer : public VtHandle<_vt::CommandBuffer> {
    public:
        operator VkCommandBuffer() const;
    };

    class VtPipelineLayout : public VtHandle<_vt::PipelineLayout> {
    public:
        operator VkPipelineLayout() const;
    };

    class VtVertexInputSet : public VtHandle<_vt::VertexInputSet> {
    public:
        operator VkDescriptorSet() const;
    };

    class VtRayTracingSet : public VtHandle<_vt::RayTracingSet> {
    public:
        operator VkDescriptorSet() const;
    };

    class VtAcceleratorSet : public VtHandle<_vt::AcceleratorSet> {
    public:
        operator VkDescriptorSet() const;
    };

    class VtVertexAssemblySet : public VtHandle<_vt::VertexAssemblySet> {
    public:
        operator VkDescriptorSet() const;
    };

    class VtMaterialSet : public VtHandle<_vt::MaterialSet> {
    public:
        operator VkDescriptorSet() const;
    };




    // handlers can't have base classes
    template<VmaMemoryUsage U>
    class VtRoledBuffer : public VtHandle<_vt::RoledBuffer<U>> {
    private: using P = VtHandle<_vt::RoledBuffer<U>>;
    public:
        operator VkBuffer() const;
        operator VkBuffer&();

        operator VkBufferView() const;
        operator VkBufferView&();
    };

    // templated implementations
    template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBuffer() const { return *P::_vtHandle; };
    template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBuffer&() { return *P::_vtHandle; };
    template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBufferView() const { return *P::_vtHandle; };
    template<VmaMemoryUsage U> inline VtRoledBuffer<U>::operator VkBufferView&() { return *P::_vtHandle; };



    class VtDeviceImage : public VtHandle<_vt::DeviceImage> {
    public:
        operator VkImage() const; //{ return *_vtHandle; };
        operator VkImageView() const; //{ return *_vtHandle; };
        operator VkImage&(); //{ return *_vtHandle; };
        operator VkImageView&(); //{ return *_vtHandle; };
    };

    class VtDevice : public VtHandle<_vt::Device> {
    public:
        operator VkDevice() const; //{ return *_vtHandle; }
        operator VkPipelineCache() const; //{ return *_vtHandle; };
        operator VkDescriptorPool() const; //{ return *_vtHandle; };

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        operator VmaAllocator() const; //{ return *_vtHandle; }
#endif
        // casting operators with traffic buffers
        operator VtHostToDeviceBuffer() const; //{ return VtHostToDeviceBuffer{ _vtHandle->_bufferTraffic->_uploadBuffer }; };
        operator VtDeviceToHostBuffer() const; //{ return VtDeviceToHostBuffer{ _vtHandle->_bufferTraffic->_downloadBuffer }; };

        // getter of descriptor layout from device VtDevice
#ifdef VRT_ENABLE_STRING_VIEW
        VkDescriptorSetLayout getDescriptorLayout(const std::string_view& name) const;
#else
        VkDescriptorSetLayout getDescriptorLayout(const std::string& name) const;
#endif
    };

};
