#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"
#include "HardClassesDef.inl"
#include "HandlersDef.inl" // unusable without definitions
#include "Enums.inl"

// C++ hard interfaces (which will storing)
namespace _vt { // store in undercover namespace
    using namespace vt;



    // ray tracing instance aggregation
    class Instance : public std::enable_shared_from_this<Instance> {
    public:
        VkInstance _instance = nullptr;

        operator VkInstance() const { return _instance; };
    };



    // ray tracing physical device handle
    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    public:
        friend Instance;
        VkPhysicalDevice _physicalDevice = nullptr;
        std::weak_ptr<Instance> _instance;

        operator VkPhysicalDevice() const { return _physicalDevice; };
        std::shared_ptr<Instance> _parent() const { return _instance.lock(); };
    };



    // ray tracing device with aggregation
    class Device : public std::enable_shared_from_this<Device> {
    public:
        friend PhysicalDevice;
        VkDevice _device = nullptr;
        std::weak_ptr<PhysicalDevice> _physicalDevice;

        VmaAllocator _allocator;
        VkPipelineCache _pipelineCache; // store native pipeline cache
        VkDescriptorPool _descriptorPool;
        std::shared_ptr<RadixSort> _radixSort; // create native radix sort
        //std::shared_ptr<CopyProgram> _copyProgram; // create native pipelines for (indirect) copying
        std::shared_ptr<HostToDeviceBuffer> _uploadBuffer; // from host
        std::shared_ptr<DeviceToHostBuffer> _downloadBuffer; // to host
        std::map<std::string, VkDescriptorSetLayout> _descriptorLayoutMap; // descriptor layout map in ray tracing system

        // weak pointers with in-device bind allocatable objects
        // make sure that these buffers stil may destoyed
        std::vector<std::weak_ptr<DeviceToHostBuffer>> _deviceToHostBuffersPtrs;
        std::vector<std::weak_ptr<HostToDeviceBuffer>> _hostToDeviceBuffersPtrs;
        std::vector<std::weak_ptr<DeviceBuffer>> _deviceBuffersPtrs;
        std::vector<std::weak_ptr<DeviceImage>> _deviceImagesPtrs;

        operator VkDevice() const { return _device; };
        operator VkPipelineCache() const { return _pipelineCache; };
        operator VkDescriptorPool() const { return _descriptorPool; };
        operator VmaAllocator() const { return _allocator; };
        std::shared_ptr<PhysicalDevice> _parent() const { return _physicalDevice.lock(); };
    };



    // ray tracing command buffer interface aggregator
    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    public:
        friend Device;
        VkCommandBuffer _commandBuffer = nullptr;
        std::weak_ptr<Device> _device;

        std::shared_ptr<MaterialSet> _currentMaterialSet; // will bound in "cmdDispatch" 
        std::shared_ptr<Accelerator> _currentAccelerator;
        std::shared_ptr<Pipeline> _currentRTPipeline;
        //std::map<uint32_t, VtVertexDataBufferBinding> _vertexDataBufferBindingMap; // for accelerator vertex building command cache
        //std::vector<VkDescriptorSet> _tmpCopyInstanceDescriptorSets; // when command buffer will submitted, prefer clean up

        operator VkCommandBuffer() const { return _commandBuffer; };
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };



    // ray tracing advanced pipeline layout
    class PipelineLayout : public std::enable_shared_from_this<PipelineLayout> {
    public:
        friend Device;
        VkPipelineLayout _pipelineLayout = nullptr; // replaced set 0 and 1
        std::weak_ptr<Device> _device;

        operator VkPipelineLayout() const { return _pipelineLayout; }; // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };



    // ray tracing advanced pipeline
    class Pipeline: public std::enable_shared_from_this<Pipeline> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting

        std::weak_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout; // customized pipeline layout, when pipeline was created
        VkPipeline _closestHitPipeline, _missHitPipeline, _generationPipeline;

        // native descriptor set
        VkDescriptorSet _rayTracingDescriptorSet;
        // material and accelerator descriptor sets, that sets to "1" is dedicated by another natives
        std::vector<VkDescriptorSet> _userDefinedDescriptorSets; // beyond than 1 only

        operator VkPipeline() const { return _dullPipeline; };
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };


    // ray tracing accelerator structure object
    // planned to merge pipeline programs to device
    class Accelerator: public std::enable_shared_from_this<Accelerator> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::weak_ptr<Device> _device;

        // traverse pipeline
        VkPipeline _intersectionPipeline;

        // vertex input assembly stage
        VkPipeline _vertexAssemblyPipeline;

        // build BVH stages (few stages, in sequences)
        VkPipeline _boundingPipeline, _shorthandPipeline, _leafPipeline, /*...radix sort between*/ _buildPipeline, _fitPipeline;

        // static pipeline layout for stages 
        VkPipelineLayout _vertexAssemblyPipelineLayout, _buildPipelineLayout, _traversePipelineLayout;

        // descritor sets for traversing, building, and vertex assembly
        VkDescriptorSet _vertexAssemblyDescriptorSet, _buildDescriptorSet, _traverseDescriptorSet;


        // internal buffers
        std::shared_ptr<DeviceBuffer> _mortonCodesBuffer, _mortonIndicesBuffer, _leafBuffer, _boundaryResultBuffer;

        // vertex and bvh export 
        std::shared_ptr<DeviceImage> _attributeTexelBuffer;
        std::shared_ptr<DeviceBuffer> _verticeBuffer, _materialBuffer, _orderBuffer, _bvhMetaBuffer, bvhBoxBuffer, _bvhBlockUniform;

        // input of vertex source data
        std::vector<std::shared_ptr<VertexInputSet>> _vertexInputs;

        operator VkPipeline() const { return _dullPipeline; };
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };



    // this is wrapped advanced buffer class
    template<VmaMemoryUsage U>
    class RoledBuffer: public std::enable_shared_from_this<RoledBuffer<U>> {
    public:
        ~RoledBuffer();

        friend Device;
        VkBuffer _buffer = nullptr;
        std::weak_ptr<Device> _device;

        VkBufferView _bufferView;
        VmaAllocation _allocation;
        VmaAllocationInfo _allocationInfo;
        VkDeviceSize _size;
        auto _hostMapped() const { return _allocationInfo.pMappedData; };

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkBuffer() const { return _buffer; }; // cast operator
        operator VkBufferView() const { return _bufferView; }; // cast operator
        VkDescriptorBufferInfo _descriptorInfo() const; //generated structure
    };



    // this is wrapped advanced image class
    class DeviceImage: public std::enable_shared_from_this<DeviceImage> {
    public:
        ~DeviceImage();

        friend Device;
        VkImage _image = nullptr;
        std::weak_ptr<Device> _device;

        VkImageView _imageView;
        VmaAllocation _allocation;
        VmaAllocationInfo _allocationInfo;
        VkImageSubresourceRange _subresourceRange;
        VkImageSubresourceLayers _subresourceLayers;
        VkImageLayout _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, _layout = VK_IMAGE_LAYOUT_GENERAL;
        VkFormat _format = VK_FORMAT_R32G32B32A32_SFLOAT;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkImage() const { return _image; }; // cast operator
        operator VkImageView() const { return _imageView; }; // cast operator
        VkDescriptorImageInfo _descriptorInfo() const; //generated structure
    };



    // this class does not using in ray tracing API
    // can be pinned with device
    class RadixSort: public std::enable_shared_from_this<RadixSort> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::weak_ptr<Device> _device;
        
        std::shared_ptr<DeviceBuffer> _stepsBuffer; // constant buffer
        std::shared_ptr<DeviceBuffer> _tmpKeysBuffer; // cache keys between stages (avoid write conflict)
        std::shared_ptr<DeviceBuffer> _tmpValuesBuffer; // cache values between stages (avoid write conflict)
        VkPipeline _histogramPipeline, _workPrefixPipeline, _permutePipeline; // radix sort pipelines
        VkPipelineLayout _pipelineLayout; // use unified pipeline layout
        
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkPipeline() const { return _dullPipeline; };
    };


    // this class does not using in ray tracing API
    // can be pinned with device 
    // in every copy procedure prefer create own descriptor sets
    // or use push descriptors 
    class CopyProgram: public std::enable_shared_from_this<CopyProgram> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::weak_ptr<Device> _device;

        VkPipeline _bufferCopyPipeline, _bufferCopyIndirectPipeline, _imageCopyPipeline, _imageCopyIndirectPipeline;
        VkPipelineLayout _bufferCopyPipelineLayout, _imageCopyPipelineLayout;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkPipeline() const { return _dullPipeline; };
    };



    class MaterialSet : public std::enable_shared_from_this<MaterialSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = nullptr;
        std::weak_ptr<Device> _device;

        // textures and samplers bound in descriptor set directly

        // material data buffers
        std::shared_ptr<DeviceBuffer> _virtualSamplerCombined;
        std::shared_ptr<DeviceBuffer> _materialDataBuffer;

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };



    class VertexInputSet : public std::enable_shared_from_this<VertexInputSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = nullptr;
        std::weak_ptr<Device> _device;

        // buffer pointers for storing vertexInput
        uint32_t _primitiveCount = 0; // for simplify of measuring when building hierarchies
        uint32_t _constPageID = 0; // if uniform store in one buffer (unused at now)
        std::shared_ptr<DeviceBuffer> _bBufferRegionBindings;
        std::shared_ptr<DeviceBuffer> _bBufferAccessors;
        std::shared_ptr<DeviceBuffer> _bBufferAttributeBindings;
        std::shared_ptr<DeviceBuffer> _bBufferViews;
        std::shared_ptr<DeviceBuffer> _constBufferUniform; // replacement for push constant (contains primitiveCount, verticeAccessorID, indiceAccessorID, materialID)
        std::shared_ptr<DeviceBuffer> _dataSourceBuffer; // universe buffer for vertex input

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };


};