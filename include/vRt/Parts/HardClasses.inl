#pragma once
#include "Headers.inl"
#include "StructuresLow.inl"
#include "HardClassesDef.inl"
#include "HandlersDef.inl" // unusable without definitions
#include "Enums.inl"

// C++ hard interfaces (which will storing)
namespace _vt { // store in undercover namespace
    using namespace vrt;

    // ray tracing instance aggregation
    class Instance : public std::enable_shared_from_this<Instance> {
    public:
        VkInstance _instance = VK_NULL_HANDLE;
        operator VkInstance() const { return _instance; };
    };

    // ray tracing physical device handle
    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    public:
        friend Instance;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        std::shared_ptr<Instance> _instance;

        operator VkPhysicalDevice() const { return _physicalDevice; };
        auto _parent() const { return _instance; };
    };

    // host <-> device buffer traffic
    class BufferTraffic : public std::enable_shared_from_this<BufferTraffic> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        std::shared_ptr<HostToDeviceBuffer> _uploadBuffer; // from host
        std::shared_ptr<DeviceToHostBuffer> _downloadBuffer; // to host

        std::shared_ptr<DeviceBuffer> _uniformVIBuffer;
        auto _parent() const { return _device.lock(); };
    };

    // ray tracing device with aggregation
    class Device : public std::enable_shared_from_this<Device> {
    public:
        friend PhysicalDevice;
        VkDevice _device = VK_NULL_HANDLE;
        std::shared_ptr<PhysicalDevice> _physicalDevice;

        uint32_t _mainFamilyIndex = 0;
        std::string _shadersPath = "./";

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocator _allocator = {};
#endif

        VkPipelineCache _pipelineCache = VK_NULL_HANDLE; // store native pipeline cache
        VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

        std::shared_ptr<RadixSort> _radixSort;
        std::shared_ptr<AcceleratorHLBVH2> _acceleratorBuilder; // planned to rename
        std::shared_ptr<VertexAssemblyPipeline> _vertexAssembler;
        std::shared_ptr<BufferTraffic> _bufferTraffic;
        VkPipeline _dullBarrier;
        //std::shared_ptr<CopyProgram> _copyProgram;

        // descriptor layout map in ray tracing system
        std::map<std::string, VkDescriptorSetLayout> _descriptorLayoutMap;
        VtVendor _vendorName = VT_VENDOR_UNIVERSAL;

        operator VkDevice() const { return _device; };
        operator VkPipelineCache() const { return _pipelineCache; };
        operator VkDescriptorPool() const { return _descriptorPool; };

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        operator VmaAllocator() const { return _allocator; };
#endif

        operator std::shared_ptr<HostToDeviceBuffer>() const { return _bufferTraffic->_uploadBuffer; };
        operator std::shared_ptr<DeviceToHostBuffer>() const { return _bufferTraffic->_downloadBuffer; };
        auto _parent() const { return _physicalDevice; };
    };

    // ray tracing command buffer interface aggregator
    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    public:
        friend Device;
        VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;

        std::weak_ptr<RayTracingSet> _rayTracingSet;
        std::weak_ptr<MaterialSet> _materialSet; // will bound in "cmdDispatch" 
        std::weak_ptr<AcceleratorSet> _acceleratorSet;
        std::weak_ptr<VertexAssemblySet> _vertexSet;
        std::weak_ptr<Pipeline> _rayTracingPipeline;
        std::vector<std::weak_ptr<VertexInputSet>> _vertexInputs; // bound vertex input sets 
        std::vector<VkDescriptorSet> _boundDescriptorSets;
        std::vector<VkDescriptorSet> _boundVIDescriptorSets;
        std::map<uint32_t, std::vector<VkDescriptorSet>> _perVertexInputDSC;

        operator VkCommandBuffer() const { return _commandBuffer; };
        auto _parent() const { return _device; };
    };

    // ray tracing advanced pipeline layout
    class PipelineLayout : public std::enable_shared_from_this<PipelineLayout> {
    public:
        friend Device;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE; // replaced set 0 and 1
        std::shared_ptr<Device> _device;
        VtPipelineLayoutType _type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING;

        operator VkPipelineLayout() const { return _pipelineLayout; }; // no correct conversion
        auto _parent() const { return _device; };
    };

    class RayTracingSet : public std::enable_shared_from_this<RayTracingSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;

        // in-set buffers
        std::shared_ptr<DeviceBuffer> _rayBuffer, _groupIndicesBuffer, _groupIndicesBufferRead, _hitBuffer, _countersBuffer, _groupCountersBuffer, _groupCountersBufferRead, _closestHitIndiceBuffer, _missedHitIndiceBuffer, _hitPayloadBuffer, _constBuffer, _traverseCache, _blockBuffer, _rayLinkPayload, _attribBuffer;
        VtStageUniform _cuniform = {};

        operator VkDescriptorSet() const { return _descriptorSet; };
        auto _parent() const { return _device; };
    };

    // ray tracing advanced pipeline
    class Pipeline : public std::enable_shared_from_this<Pipeline> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = VK_NULL_HANDLE; // protect from stupid casting

        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout; // customized pipeline layout, when pipeline was created

        // 
        //VkPipeline _generationPipeline, _tripletPipeline, _closestHitPipeline[4], _missHitPipeline[1], _groupPipelines[4];
        std::vector<VkPipeline> _generationPipeline, _closestHitPipeline, _missHitPipeline, _groupPipelines;

        // material and accelerator descriptor sets, that sets to "1" is dedicated by another natives
        std::vector<VkDescriptorSet> _userDefinedDescriptorSets; // beyond than 1 only

        operator VkPipeline() const { return _dullPipeline; };
        auto _parent() const { return _device; };
    };

    // vertex assembly cache 
    class VertexAssemblySet : public std::enable_shared_from_this<VertexAssemblySet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;

        // vertex and bvh export 
        std::shared_ptr<DeviceImage> _attributeTexelBuffer;
        std::shared_ptr<DeviceBuffer> _verticeBuffer, _verticeBufferSide, _verticeBufferIn, _materialBuffer, _bitfieldBuffer, _countersBuffer, _normalBuffer;

        // input of vertex source data
        std::vector<std::shared_ptr<VertexInputSet>> _vertexInputs;

        // primitive count 
        uint32_t _calculatedPrimitiveCount = 0;

        operator VkDescriptorSet() const { return _descriptorSet; };
        auto _parent() const { return _device; };
    };

    // vertex assembly program
    class VertexAssemblyPipeline : public std::enable_shared_from_this<VertexAssemblyPipeline> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = VK_NULL_HANDLE; // protect from stupid casting
        std::weak_ptr<Device> _device;

        VkPipeline _vertexAssemblyPipeline = VK_NULL_HANDLE;
        std::shared_ptr<PipelineLayout> _pipelineLayout;

        operator VkPipeline() const { return _dullPipeline; };
        auto _parent() const { return _device.lock(); };
    };

    // accelerator store set
    class AcceleratorSet : public std::enable_shared_from_this<AcceleratorSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE; // protect from stupid casting
        std::shared_ptr<Device> _device;

        // vertex and bvh export 
        std::shared_ptr<DeviceBuffer> _bvhMetaBuffer, _bvhBoxBuffer, _bvhBlockUniform;
        uint32_t _entryID = 0, _primitiveCount = -1, _primitiveOffset = 0;
        VtBvhBlock _bvhBlockData;

        // build descriptor set 
        VkDescriptorSet _buildDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet _sortDescriptorSet = VK_NULL_HANDLE;

        // internal buffers
        std::shared_ptr<DeviceBuffer> _mortonCodesBuffer, _mortonIndicesBuffer, _leafBuffer, _generalBoundaryResultBuffer, _leafNodeIndices, _currentNodeIndices, _fitStatusBuffer, _countersBuffer, _onWorkBoxes;

        operator VkDescriptorSet() const { return _descriptorSet; };
        auto _parent() const { return _device; };
    };

    // ray tracing accelerator structure object
    // planned to merge pipeline programs to device
    class AcceleratorHLBVH2 : public std::enable_shared_from_this<AcceleratorHLBVH2> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = VK_NULL_HANDLE; // protect from stupid casting
        std::weak_ptr<Device> _device;

        // traverse pipeline
        VkPipeline _intersectionPipeline = VK_NULL_HANDLE, _interpolatorPipeline = VK_NULL_HANDLE;

        // build BVH stages (few stages, in sequences)
        VkPipeline _boundingPipeline = VK_NULL_HANDLE, _shorthandPipeline = VK_NULL_HANDLE, _leafPipeline = VK_NULL_HANDLE, /*...radix sort between*/ _buildPipeline = VK_NULL_HANDLE, _buildPipelineFirst = VK_NULL_HANDLE, _fitPipeline = VK_NULL_HANDLE, _leafLinkPipeline = VK_NULL_HANDLE;

        // static pipeline layout for stages 
        VkPipelineLayout _buildPipelineLayout = VK_NULL_HANDLE, _traversePipelineLayout = VK_NULL_HANDLE;

        operator VkPipeline() const { return _dullPipeline; };
        auto _parent() const { return _device.lock(); };
    };

    // this is wrapped advanced buffer class
    template<VmaMemoryUsage U>
    class RoledBuffer : public std::enable_shared_from_this<RoledBuffer<U>> {
    public:
        ~RoledBuffer();

        friend Device;
        VkBuffer _buffer = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;

        VkBufferView _bufferView = VK_NULL_HANDLE;

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocation _allocation = {};
        VmaAllocationInfo _allocationInfo = {};
#endif

        VkDeviceSize _size = 0;
        VkDescriptorBufferInfo _staticDsci = {};
        auto _hostMapped() const { return _allocationInfo.pMappedData; };

        auto _parent() const { return _device; };
        operator VkBuffer() const { return _buffer; }; // cast operator
        operator VkBufferView() const { return _bufferView; }; // cast operator

        auto _genDescriptorInfo() const { return VkDescriptorBufferInfo{ _buffer, 0u, VK_WHOLE_SIZE }; };
        auto& _descriptorInfo() const { return this->_staticDsci; };
        auto& _descriptorInfo() { this->_staticDsci = this->_genDescriptorInfo(); return this->_staticDsci; };
    };

    // this is wrapped advanced image class
    class DeviceImage : public std::enable_shared_from_this<DeviceImage> {
    public:
        ~DeviceImage();

        friend Device;
        VkImage _image = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;

        VkImageView _imageView = {};
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocation _allocation = {};
        VmaAllocationInfo _allocationInfo = {};
#endif

        VkImageSubresourceRange _subresourceRange = {};
        VkImageSubresourceLayers _subresourceLayers = {};
        VkImageLayout _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, _layout = VK_IMAGE_LAYOUT_GENERAL;
        VkFormat _format = VK_FORMAT_R32G32B32A32_UINT;
        VkExtent3D _extent = {1u, 1u, 1u};
        VkDescriptorImageInfo _staticDsci = {};

        auto _parent() const { return _device; };
        operator VkImage() const { return _image; }; // cast operator
        operator VkImageView() const { return _imageView; }; // cast operator

        auto _genDescriptorInfo() const { return VkDescriptorImageInfo{ {}, _imageView, _layout }; };
        auto& _descriptorInfo() const { return this->_staticDsci; };
        auto& _descriptorInfo() { this->_staticDsci = this->_genDescriptorInfo(); return this->_staticDsci; };
    };

    // this class does not using in ray tracing API
    // can be pinned with device
    class RadixSort : public std::enable_shared_from_this<RadixSort> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = VK_NULL_HANDLE; // protect from stupid casting
        std::shared_ptr<Device> _device;

        std::shared_ptr<DeviceBuffer> _histogramBuffer;
        std::shared_ptr<DeviceBuffer> _prefixSumBuffer;
        std::shared_ptr<DeviceBuffer> _stepsBuffer; // constant buffer
        std::shared_ptr<DeviceBuffer> _tmpKeysBuffer; // cache keys between stages (avoid write conflict)
        std::shared_ptr<DeviceBuffer> _tmpValuesBuffer; // cache values between stages (avoid write conflict)
        VkPipeline _histogramPipeline = VK_NULL_HANDLE, _workPrefixPipeline = VK_NULL_HANDLE, _permutePipeline = VK_NULL_HANDLE, _copyhackPipeline = VK_NULL_HANDLE; // radix sort pipelines
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE; // use unified pipeline layout 
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;

        auto _parent() const { return _device; };
        operator VkPipeline() const { return _dullPipeline; };
    };

    // this class does not using in ray tracing API
    // can be pinned with device 
    // in every copy procedure prefer create own descriptor sets
    // or use push descriptors 
    class CopyProgram : public std::enable_shared_from_this<CopyProgram> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = VK_NULL_HANDLE; // protect from stupid casting
        std::shared_ptr<Device> _device;

        VkPipeline _bufferCopyPipeline, _bufferCopyIndirectPipeline, _imageCopyPipeline, _imageCopyIndirectPipeline;
        VkPipelineLayout _bufferCopyPipelineLayout, _imageCopyPipelineLayout;

        auto _parent() const { return _device; };
        operator VkPipeline() const { return _dullPipeline; };
    };

    class MaterialSet : public std::enable_shared_from_this<MaterialSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;

        // textures and samplers bound in descriptor set directly

        // material data buffers
        //std::shared_ptr<DeviceBuffer> _virtualSamplerCombinedBuffer;
        //std::shared_ptr<DeviceBuffer> _materialDataBuffer;
        std::shared_ptr<DeviceBuffer> _constBuffer;

        uint32_t _materialCount = 0;
        uint32_t _materialOffset = 0;

        auto _parent() const { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };

    class VertexInputSet : public std::enable_shared_from_this<VertexInputSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
        std::shared_ptr<Device> _device;
        VtUniformBlock _uniformBlock = {};

        // vertex assembly pipeline bound
        std::shared_ptr<DeviceBuffer> _uniformBlockBuffer;
        std::shared_ptr<VertexAssemblyPipeline> _vertexAssembly;

        auto _parent() const { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };

        auto& uniform() { return _uniformBlock; };
        const auto& uniform() const { return _uniformBlock; };
    };
};