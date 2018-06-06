#pragma once
#include "Headers.inl"
#include "Enums.inl"

// C++ hard interfaces (which will storing)
namespace _vt { // store in undercover namespace
    using namespace vt;

    class Instance;
    class PhysicalDevice;
    class Device;
    class RadixSort;
    class CommandBuffer;
    class Pipeline;
    class CopyProgram;
    class VertexInput;
    class Accelerator;
    class MaterialSet;

    // use roled buffers
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY> class RoledBuffer;
    using DeviceBuffer = RoledBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    using DeviceToHostBuffer = RoledBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;
    using HostToDeviceBuffer = RoledBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;

    // have no roles at now
    class DeviceImage;



    // ray tracing instance aggregation
    class Instance : public std::enable_shared_from_this<Instance> {
    public:
        VkInstance _instance;

        operator VkInstance() const { return _instance; }
    };

    // ray tracing physical device handle
    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    public:
        friend Instance;
        std::weak_ptr<Instance> _instance;
        VkPhysicalDevice _physicalDevice;

        operator VkPhysicalDevice() const { return _physicalDevice; }
        std::shared_ptr<Instance> _parent() const { return _instance.lock(); };
    };

    // ray tracing device with aggregation
    class Device : public std::enable_shared_from_this<Device> {
    public:
        friend PhysicalDevice;
        std::weak_ptr<PhysicalDevice> _physicalDevice;
        std::shared_ptr<RadixSort> _radixSort; // create native radix sort
        std::shared_ptr<CopyProgram> _copyProgram; // create native pipelines for copying
        std::shared_ptr<HostToDeviceBuffer> _uploadBuffer; // from host
        std::shared_ptr<DeviceToHostBuffer> _downloadBuffer; // to host
        std::map<std::string, VkDescriptorSetLayout> _descriptorLayoutMap; // descriptor layout map in ray tracing system

        VmaAllocator _allocator;
        VkDevice _device;

        operator VkDevice() const { return _device; }
        std::shared_ptr<PhysicalDevice> _parent() const { return _physicalDevice.lock(); };
    };

    // ray tracing command buffer interface aggregator
    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        VkCommandBuffer _cmd;

        std::shared_ptr<MaterialSet> _materialSetTmp; // will bound in "cmdDispatch" 
        // TODO: temporary store vertex data buffers

        operator VkCommandBuffer() const { return _cmd; }
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };

    // ray tracing advanced pipeline layout (unfinished)
    class PipelineLayout : public std::enable_shared_from_this<PipelineLayout> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        VkPipelineLayout _pipelineLayout; // has blocked set 0 and 1
        
        operator VkPipelineLayout() const { return _pipelineLayout; }; // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };

    // ray tracing advanced pipeline (unfinished)
    class Pipeline: public std::enable_shared_from_this<Pipeline> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout; // customized pipeline layout
        VkPipeline _closestHitPipeline, _missHitPipeline, _generationPipeline;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };

    // ray tracing accelerator structure object (unfinished)
    class Accelerator: public std::enable_shared_from_this<Accelerator> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;

        // traverse
        VkPipeline _intersectionPipeline;

        // vertex input stage
        VkPipeline _vertexAssemblyPipeline;

        // build BVH stages (few stages, in sequences)
        VkPipeline _boundingPipeline, _shorthandPipeline, _leafPipeline, /*...radix sort between*/ _buildPipeline, _fitPipeline;

        // static pipeline layout for stages
        VkPipelineLayout _vertexAssemblyPipelineLayout, _buildPipelineLayout, _traversePipelineLayout;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };



    // this is wrapped advanced buffer class
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY>
    class RoledBuffer: public std::enable_shared_from_this<RoledBuffer<U>> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        VkBuffer _buffer;
        VkBufferView _bufferView;
        VmaAllocation _allocation;
        VmaAllocationInfo _allocationInfo;
        VkDeviceSize _size;
        auto _hostMapped() const { return _allocationInfo.pMappedData; }

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkBuffer() const { return _buffer; } // cast operator
        operator VkBufferView() const { return _bufferView; } // cast operator
        VkDescriptorBufferInfo _descriptorInfo(); //generated structure
    };


    // this is wrapped advanced image class
    class DeviceImage: public std::enable_shared_from_this<DeviceImage> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        VkImage _image;
        VkImageView _imageView;
        VmaAllocation _allocation;
        VmaAllocationInfo _allocationInfo;
        VkImageSubresourceRange _subresourceRange;
        VkImageSubresourceLayers _subresourceLayers;
        VkImageLayout _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, _layout = VK_IMAGE_LAYOUT_GENERAL;
        VkFormat _format = VK_FORMAT_R32G32B32A32_SFLOAT;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkImage() const { return _image; } // cast operator
        operator VkImageView() const { return _imageView; } // cast operator
        VkDescriptorImageInfo _descriptorInfo(); //generated structure
    };






    // this class does not using in ray tracing API
    // can be pinned with device
    class RadixSort: public std::enable_shared_from_this<RadixSort> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        std::shared_ptr<DeviceBuffer> _stepsBuffer; // constant buffer
        std::shared_ptr<DeviceBuffer> _tmpKeysBuffer; // cache keys between stages (avoid write conflict)
        std::shared_ptr<DeviceBuffer> _tmpValuesBuffer; // cache values between stages (avoid write conflict)
        VkPipeline _histogramPipeline, _workPrefixPipeline, _permutePipeline; // radix sort pipelines
        VkPipelineLayout _pipelineLayout; // use unified pipeline layout
        
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };

    // this class does not using in ray tracing API
    // can be pinned with device
    class CopyProgram: public std::enable_shared_from_this<CopyProgram> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        VkPipeline _bufferCopyPipeline, _bufferCopyIndirectPipeline, _imageCopyPipeline, _imageCopyIndirectPipeline;
        VkPipelineLayout _bufferCopyPipelineLayout, _imageCopyPipelineLayout;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };


    class MaterialSet : public std::enable_shared_from_this<MaterialSet> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        VkDescriptorSet _descriptorSet;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };

};