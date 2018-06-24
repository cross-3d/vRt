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
        std::shared_ptr<Instance> _instance;

        operator VkPhysicalDevice() const { return _physicalDevice; };
        std::shared_ptr<Instance> _parent() const { return _instance; };
    };



    // host <-> device buffer traffic
    class BufferTraffic : public std::enable_shared_from_this<BufferTraffic> {
    public:
        friend Device;
        std::weak_ptr<Device> _device;
        std::shared_ptr<HostToDeviceBuffer> _uploadBuffer; // from host
        std::shared_ptr<DeviceToHostBuffer> _downloadBuffer; // to host
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };


    // ray tracing device with aggregation
    class Device : public std::enable_shared_from_this<Device> {
    public:
        friend PhysicalDevice;
        VkDevice _device = nullptr;
        std::shared_ptr<PhysicalDevice> _physicalDevice;

        uint32_t _mainFamilyIndex = 0;
        std::string _shadersPath = "./";
        VmaAllocator _allocator;
        VkPipelineCache _pipelineCache; // store native pipeline cache
        VkDescriptorPool _descriptorPool;

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
        operator VmaAllocator() const { return _allocator; };
        operator std::shared_ptr<HostToDeviceBuffer>() const { return _bufferTraffic->_uploadBuffer; };
        operator std::shared_ptr<DeviceToHostBuffer>() const { return _bufferTraffic->_downloadBuffer; };
        std::shared_ptr<PhysicalDevice> _parent() const { return _physicalDevice; };
    };



    // ray tracing command buffer interface aggregator
    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    public:
        friend Device;
        VkCommandBuffer _commandBuffer = nullptr;
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
        std::shared_ptr<Device> _parent() const { return _device; };
    };



    // ray tracing advanced pipeline layout
    class PipelineLayout : public std::enable_shared_from_this<PipelineLayout> {
    public:
        friend Device;
        VkPipelineLayout _pipelineLayout = nullptr; // replaced set 0 and 1
        std::shared_ptr<Device> _device;
        VtPipelineLayoutType _type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING;

        operator VkPipelineLayout() const { return _pipelineLayout; }; // no correct conversion
        std::shared_ptr<Device> _parent() const { return _device; };
    };



    struct VtStageUniform { int width = 1, height = 1, iteration = 0, closestHitOffset = 0; };

    class RayTracingSet : public std::enable_shared_from_this<RayTracingSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = nullptr;
        std::shared_ptr<Device> _device;

        // in-set buffers
        std::shared_ptr<DeviceBuffer> _rayBuffer, _rayIndiceBuffer, _hitBuffer, _countersBuffer, _closestHitIndiceBuffer, _missedHitIndiceBuffer, _hitPayloadBuffer, _constBuffer, _traverseCache, _blockBuffer, _rayLinkPayload, _attribBuffer;
        VtStageUniform _cuniform;

        operator VkDescriptorSet() const { return _descriptorSet; };
        std::shared_ptr<Device> _parent() const { return _device; };
    };


    // ray tracing advanced pipeline
    class Pipeline : public std::enable_shared_from_this<Pipeline> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting

        std::shared_ptr<Device> _device;
        std::shared_ptr<PipelineLayout> _pipelineLayout; // customized pipeline layout, when pipeline was created

        // 
        VkPipeline _generationPipeline, _closestHitPipeline, _missHitPipeline, _resolvePipeline;

        // material and accelerator descriptor sets, that sets to "1" is dedicated by another natives
        std::vector<VkDescriptorSet> _userDefinedDescriptorSets; // beyond than 1 only

        operator VkPipeline() const { return _dullPipeline; };
        std::shared_ptr<Device> _parent() const { return _device; };
    };


    // vertex assembly cache 
    class VertexAssemblySet : public std::enable_shared_from_this<VertexAssemblySet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = nullptr;
        std::shared_ptr<Device> _device;

        // vertex and bvh export 
        std::shared_ptr<DeviceImage> _attributeTexelBuffer;
        std::shared_ptr<DeviceBuffer> _verticeBuffer, _verticeBufferIn, _materialBuffer, _orderBuffer, _countersBuffer;

        // input of vertex source data
        std::vector<std::shared_ptr<VertexInputSet>> _vertexInputs;

        // primitive count 
        uint32_t _calculatedPrimitiveCount = 0;

        operator VkDescriptorSet() const { return _descriptorSet; };
        std::shared_ptr<Device> _parent() const { return _device; };
    };


    // vertex assembly program
    class VertexAssemblyPipeline : public std::enable_shared_from_this<VertexAssemblyPipeline> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::weak_ptr<Device> _device;

        VkPipeline _vertexAssemblyPipeline;
        std::shared_ptr<PipelineLayout> _pipelineLayout;

        operator VkPipeline() const { return _dullPipeline; };
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };



    

    struct VtBvhBlock {
        VtMat4 transform;
        VtMat4 transformInv;
        VtMat4 projection;
        VtMat4 projectionInv;
        int leafCount = 0, primitiveCount = 0, entryID = 0, primitiveOffset = 0;
    };

    // accelerator store set
    class AcceleratorSet : public std::enable_shared_from_this<AcceleratorSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet; // protect from stupid casting
        std::shared_ptr<Device> _device;

        // vertex and bvh export 
        std::shared_ptr<DeviceBuffer> _bvhMetaBuffer, _bvhBoxBuffer, _bvhBlockUniform;
        uint32_t _entryID = 0, _primitiveCount = -1, _primitiveOffset = 0;
        VtBvhBlock _bvhBlockData;

        // build descriptor set 
        VkDescriptorSet _buildDescriptorSet;
        VkDescriptorSet _sortDescriptorSet;

        // internal buffers
        std::shared_ptr<DeviceBuffer> _mortonCodesBuffer, _mortonIndicesBuffer, _leafBuffer, _generalBoundaryResultBuffer, _leafNodeIndices, _currentNodeIndices, _fitStatusBuffer, _countersBuffer, _onWorkBoxes;

        operator VkDescriptorSet() const { return _descriptorSet; };
        std::shared_ptr<Device> _parent() const { return _device; };
    };


    // ray tracing accelerator structure object
    // planned to merge pipeline programs to device
    class AcceleratorHLBVH2 : public std::enable_shared_from_this<AcceleratorHLBVH2> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::weak_ptr<Device> _device;

        // traverse pipeline
        VkPipeline _intersectionPipeline, _interpolatorPipeline;

        // build BVH stages (few stages, in sequences)
        VkPipeline _boundingPipeline, _shorthandPipeline, _leafPipeline, /*...radix sort between*/ _buildPipeline, _fitPipeline, _leafLinkPipeline;

        // static pipeline layout for stages 
        VkPipelineLayout _buildPipelineLayout, _traversePipelineLayout;


        // build descriptor set 
        //VkDescriptorSet _buildDescriptorSet;
        //VkDescriptorSet _sortDescriptorSet;

        // internal buffers
        //std::shared_ptr<DeviceBuffer> _mortonCodesBuffer, _mortonIndicesBuffer, _leafBuffer, _generalBoundaryResultBuffer, _leafNodeIndices, _currentNodeIndices, _fitStatusBuffer, _countersBuffer, _onWorkBoxes;



        operator VkPipeline() const { return _dullPipeline; };
        std::shared_ptr<Device> _parent() const { return _device.lock(); };
    };




    // this is wrapped advanced buffer class
    template<VmaMemoryUsage U>
    class RoledBuffer : public std::enable_shared_from_this<RoledBuffer<U>> {
    public:
        ~RoledBuffer();

        friend Device;
        VkBuffer _buffer = nullptr;
        std::shared_ptr<Device> _device;

        VkBufferView _bufferView;
        VmaAllocation _allocation;
        VmaAllocationInfo _allocationInfo;
        VkDeviceSize _size;
        auto _hostMapped() const { return _allocationInfo.pMappedData; };

        std::shared_ptr<Device> _parent() const { return _device.lock(); };
        operator VkBuffer() const { return _buffer; }; // cast operator
        operator VkBufferView() const { return _bufferView; }; // cast operator
        auto _descriptorInfo() const {
            return VkDescriptorBufferInfo{ _buffer, 0u, VK_WHOLE_SIZE };
        };
    };



    // this is wrapped advanced image class
    class DeviceImage : public std::enable_shared_from_this<DeviceImage> {
    public:
        ~DeviceImage();

        friend Device;
        VkImage _image = nullptr;
        std::shared_ptr<Device> _device;

        VkImageView _imageView;
        VmaAllocation _allocation;
        VmaAllocationInfo _allocationInfo;
        VkImageSubresourceRange _subresourceRange;
        VkImageSubresourceLayers _subresourceLayers;
        VkImageLayout _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, _layout = VK_IMAGE_LAYOUT_GENERAL;
        VkFormat _format = VK_FORMAT_R32G32B32A32_SFLOAT;
        VkExtent3D _extent = {1u, 1u, 1u};

        std::shared_ptr<Device> _parent() const { return _device; };
        operator VkImage() const { return _image; }; // cast operator
        operator VkImageView() const { return _imageView; }; // cast operator
        auto _descriptorInfo() const {
            return VkDescriptorImageInfo{ {}, _imageView, _layout };
        };
    };



    // this class does not using in ray tracing API
    // can be pinned with device
    class RadixSort : public std::enable_shared_from_this<RadixSort> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::shared_ptr<Device> _device;

        std::shared_ptr<DeviceBuffer> _histogramBuffer;
        std::shared_ptr<DeviceBuffer> _prefixSumBuffer;
        std::shared_ptr<DeviceBuffer> _stepsBuffer; // constant buffer
        std::shared_ptr<DeviceBuffer> _tmpKeysBuffer; // cache keys between stages (avoid write conflict)
        std::shared_ptr<DeviceBuffer> _tmpValuesBuffer; // cache values between stages (avoid write conflict)
        VkPipeline _histogramPipeline, _workPrefixPipeline, _permutePipeline, _copyhackPipeline; // radix sort pipelines
        VkPipelineLayout _pipelineLayout; // use unified pipeline layout 
        VkDescriptorSet _descriptorSet;

        std::shared_ptr<Device> _parent() const { return _device; };
        operator VkPipeline() const { return _dullPipeline; };
    };


    // this class does not using in ray tracing API
    // can be pinned with device 
    // in every copy procedure prefer create own descriptor sets
    // or use push descriptors 
    class CopyProgram : public std::enable_shared_from_this<CopyProgram> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = nullptr; // protect from stupid casting
        std::shared_ptr<Device> _device;

        VkPipeline _bufferCopyPipeline, _bufferCopyIndirectPipeline, _imageCopyPipeline, _imageCopyIndirectPipeline;
        VkPipelineLayout _bufferCopyPipelineLayout, _imageCopyPipelineLayout;

        std::shared_ptr<Device> _parent() const { return _device; };
        operator VkPipeline() const { return _dullPipeline; };
    };



    class MaterialSet : public std::enable_shared_from_this<MaterialSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = nullptr;
        std::shared_ptr<Device> _device;

        // textures and samplers bound in descriptor set directly

        // material data buffers
        //std::shared_ptr<DeviceBuffer> _virtualSamplerCombinedBuffer;
        //std::shared_ptr<DeviceBuffer> _materialDataBuffer;
        std::shared_ptr<DeviceBuffer> _constBuffer;

        uint32_t _materialCount = 0;
        uint32_t _materialOffset = 0;

        std::shared_ptr<Device> _parent() const { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };



    struct VtUniformBlock {
        uint32_t primitiveCount = 0;
        uint32_t verticeAccessor = 0;
        uint32_t indiceAccessor = 0xFFFFFFFFu;
        uint32_t materialID = 0;

        uint32_t primitiveOffset = 0;
        uint32_t topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;
        uint32_t attributeCount = 8;
        uint32_t inputID = 0;

        uint32_t materialAccessor = 0;
        uint32_t updateOnly = 0;
        uint32_t readOffset = 0;
        uint32_t reserved2 = 0;
    };



    class VertexInputSet : public std::enable_shared_from_this<VertexInputSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = nullptr;
        std::shared_ptr<Device> _device;
        VtUniformBlock _uniformBlock;

        // buffer pointers for storing vertexInput

        //std::shared_ptr<DeviceBuffer> _bBufferRegionBindings;
        //std::shared_ptr<DeviceBuffer> _bBufferAccessors;
        //std::shared_ptr<DeviceBuffer> _bBufferAttributeBindings;
        //std::shared_ptr<DeviceBuffer> _bBufferViews;
        std::shared_ptr<DeviceBuffer> _uniformBlockBuffer; // replacement for push constant (contains primitiveCount, verticeAccessorID, indiceAccessorID, materialID)
        //std::shared_ptr<DeviceBuffer> _dataSourceBuffer; // universe buffer for vertex input

        // vertex assembly set
        //std::shared_ptr<VertexAssemblySet> _vertexAssemblySet;

        // vertex assembly pipeline bound
        std::shared_ptr<VertexAssemblyPipeline> _vertexAssembly;

        std::shared_ptr<Device> _parent() const { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };


};