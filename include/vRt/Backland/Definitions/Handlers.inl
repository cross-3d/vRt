#pragma once

// implementable
#include "HardClassesDef.inl"
//#include "HardClasses.inl"

// implement of handlers
namespace vrt { // store in official namespace

    // unified template for making regular classes
    template<class T>
    class VtHandle {
    protected:
        std::shared_ptr<T> _vtHandle = {};
    public:
        //VtHandle(std::shared_ptr<T> _H = {}) : _vtHandle(_H) {}; // constructor
        //VtHandle(const std::shared_ptr<_vt::BufferRegion>& _H) : _vtHandle(_H) {};
        VtHandle(std::shared_ptr<T>& _H) : _vtHandle(_H) {}; // reference
        VtHandle(std::shared_ptr<T>&& _H = {}) : _vtHandle(_H) {}; // initializer
        VtHandle(const std::shared_ptr<T>& _H) : _vtHandle(_H) {}; // const reference
        auto* operator->() { return _vtHandle.get(); };
        auto* operator->() const { return _vtHandle.get(); };
        operator const T&() const { return *_vtHandle; };
        operator bool() const { return !!_vtHandle; };
        //operator std::weak_ptr<T>&&() const { return _vtHandle; }; // may be ambiguous
        operator const std::shared_ptr<T>&() const { return _vtHandle; }; // experimental explicit casting
        operator std::shared_ptr<T>&() { return _vtHandle; };
        //operator T&() { return *_vtHandle; };
    };



    class VtPipeline : public VtHandle<_vt::Pipeline> {};
    class VtAcceleratorHLBVH2 : public VtHandle<_vt::AcceleratorHLBVH2> {};
    class VtAttributePipeline : public VtHandle<_vt::AssemblyPipeline> {};

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
    template<VtMemoryUsage U>
    class VtRoledBuffer : public VtHandle<_vt::RoledBuffer<U>> {
    private:
        using P = VtHandle<_vt::RoledBuffer<U>>;
        using T = std::shared_ptr<_vt::RoledBuffer<U>>;
    public:
        operator const VkBuffer&() const;
        operator VkBuffer&();

        operator const VkBufferView&() const;
        operator VkBufferView&();
    };

    // templated implementations
    template<VtMemoryUsage U> inline VtRoledBuffer<U>::operator const VkBuffer&() const { return *P::_vtHandle; };
    template<VtMemoryUsage U> inline VtRoledBuffer<U>::operator VkBuffer&() { return *P::_vtHandle; };
    template<VtMemoryUsage U> inline VtRoledBuffer<U>::operator const VkBufferView&() const { return *P::_vtHandle; };
    template<VtMemoryUsage U> inline VtRoledBuffer<U>::operator VkBufferView&() { return *P::_vtHandle; };


    class VtDeviceImage : public VtHandle<_vt::DeviceImage> {
    public:
        operator const VkImage&() const; //{ return *_vtHandle; };
        operator const VkImageView&() const; //{ return *_vtHandle; };
        operator VkImage&(); //{ return *_vtHandle; };
        operator VkImageView&(); //{ return *_vtHandle; };
    };

    class VtDevice : public VtHandle<_vt::Device> {
    public:
        operator VkDevice() const; //{ return *_vtHandle; }
        operator VkPipelineCache() const; //{ return *_vtHandle; };
        operator VkDescriptorPool() const; //{ return *_vtHandle; };

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        explicit operator VmaAllocator() const; //{ return *_vtHandle; }
#endif

        const void* _getAllocator() const;

        // getter of descriptor layout from device VtDevice
#ifdef VRT_ENABLE_STRING_VIEW
        VkDescriptorSetLayout getDescriptorLayout(const std::string_view& name) const;
#else
        VkDescriptorSetLayout getDescriptorLayout(const std::string& name) const;
#endif
    };

};
