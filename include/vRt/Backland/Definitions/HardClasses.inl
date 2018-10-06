#pragma once

// implementable weak classes
#include "HardClassesDef.inl"

// C++ hard interfaces (which will storing)
namespace _vt { // store in undercover namespace
    using namespace vrt;

    // ray tracing instance aggregation
    class Instance : public std::enable_shared_from_this<Instance> {
    public:
        VkInstance _instance = {};
        operator VkInstance() const { return _instance; };
    };

    // ray tracing physical device handle
    class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice> {
    public:
        friend Instance;
        VkPhysicalDevice _physicalDevice = {};
        std::shared_ptr<Instance> _instance = {};

        operator VkPhysicalDevice() const { return _physicalDevice; };
        auto _parent() const { return _instance; };
        auto& _parent() { return _instance; };
    };

    // host <-> device buffer traffic
    class BufferTraffic : public std::enable_shared_from_this<BufferTraffic> {
    public:
        friend Device;
        std::weak_ptr<Device> _device = {};
        std::shared_ptr<HostToDeviceBuffer> _uploadBuffer = {}; // from host
        std::shared_ptr<DeviceToHostBuffer> _downloadBuffer = {}; // to host
        std::shared_ptr<DeviceBuffer> _uniformVIBuffer = {};
    };

    // advanced device features
    class DeviceFeatures : public std::enable_shared_from_this<DeviceFeatures> {
        public:
        friend Device;

        // mainline features
        VkPhysicalDeviceFeatures2 _features = {};
        
        // device linking
        std::shared_ptr<PhysicalDevice> _physicalDevice = {};
        std::weak_ptr<Device> _device = {};

        // extensions
        VkPhysicalDevice16BitStorageFeatures _storage16 = {};
        VkPhysicalDevice8BitStorageFeaturesKHR _storage8 = {};
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT _descriptorIndexing = {};

        // features linking
        operator VkPhysicalDeviceFeatures2&() { return _features; };
        operator VkPhysicalDeviceFeatures2() const { return _features; };

        auto _parent() const { return _physicalDevice; };
        auto& _parent() { return _physicalDevice; };
    };

    // ray tracing device with aggregation
    class Device : public std::enable_shared_from_this<Device> {
    public:
        ~Device(){ this->_device = {}; };

        friend PhysicalDevice;
        VkDevice _device = {};
        std::shared_ptr<DeviceFeatures> _features = {};
        std::shared_ptr<PhysicalDevice> _physicalDevice = {};
        std::shared_ptr<AdvancedAcceleratorBase> _advancedAccelerator = {};

        uint32_t _mainFamilyIndex = 0;
        std::string _shadersPath = "./";

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocator _allocator = {};
        explicit operator VmaAllocator() const { return _allocator; };
#endif

        VkPipelineCache _pipelineCache = {}; // store native pipeline cache
        VkDescriptorPool _descriptorPool = {};

        std::shared_ptr<RadixSort> _radixSort = {};
        std::shared_ptr<AcceleratorHLBVH2> _acceleratorBuilder = {}; // planned to rename
        std::shared_ptr<AssemblyPipeline> _nativeVertexAssembler = {};
        std::shared_ptr<BufferTraffic> _bufferTraffic = {};
        VkPipeline _dullBarrier = {};

        // descriptor layout map in ray tracing system
        std::map<std::string, VkDescriptorSetLayout> _descriptorLayoutMap = {};
        VtVendor _vendorName = VT_VENDOR_UNIVERSAL;

        operator VkDevice() const { return _device; };
        operator VkPipelineCache() const { return _pipelineCache; };
        operator VkDescriptorPool() const { return _descriptorPool; };

        operator std::shared_ptr<HostToDeviceBuffer>() const { return _bufferTraffic->_uploadBuffer; };
        operator std::shared_ptr<DeviceToHostBuffer>() const { return _bufferTraffic->_downloadBuffer; };

        auto _parent() const { return _physicalDevice; };
        auto& _parent() { return _physicalDevice; };
    };

    // ray tracing command buffer interface aggregator
    class CommandBuffer : public std::enable_shared_from_this<CommandBuffer> {
    public:
        friend Device;
        VkCommandBuffer _commandBuffer = {};
        std::shared_ptr<Device> _device = {};

        std::weak_ptr<RayTracingSet> _rayTracingSet = {};
        std::weak_ptr<MaterialSet> _materialSet = {}; // will bound in "cmdDispatch"
        std::weak_ptr<AcceleratorSet> _acceleratorSet = {};
        std::weak_ptr<VertexAssemblySet> _vertexSet = {};
        std::weak_ptr<Pipeline> _rayTracingPipeline = {};
        
        std::vector<std::shared_ptr<VertexInputSet>> _vertexInputs = {}; // bound vertex input sets
        std::vector<VkDescriptorSet> _boundDescriptorSets = {};
        std::vector<VkDescriptorSet> _boundVIDescriptorSets = {};
        std::map<uint32_t, std::vector<VkDescriptorSet>> _perVertexInputDSC = {};

        operator VkCommandBuffer() const { return _commandBuffer; };

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    // ray tracing advanced pipeline layout
    class PipelineLayout : public std::enable_shared_from_this<PipelineLayout> {
    public:
        friend Device;
        VkPipelineLayout _pipelineLayout = {}; // replaced set 0 and 1
        std::shared_ptr<Device> _device = {};
        VtPipelineLayoutType _type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING;

        operator VkPipelineLayout() const { return _pipelineLayout; }; // no correct conversion

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    
    class RayTracingSet : public std::enable_shared_from_this<RayTracingSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};

        // in-set buffers
        //std::shared_ptr<DeviceBuffer> 
        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};
        std::shared_ptr<BufferRegion> 
            _rayBuffer = {},
            _groupIndicesBuffer = {},
            _groupIndicesBufferRead = {},
            _hitBuffer, _countersBuffer = {},
            _groupCountersBuffer = {},
            _groupCountersBufferRead = {},
            _closestHitIndiceBuffer = {},
            _missedHitIndiceBuffer = {},
            _hitPayloadBuffer = {},
            _constBuffer = {},
            _traverseCache = {},
            _blockBuffer = {},
            _rayLinkPayload = {},
            _attribBuffer = {};
        VtStageUniform _cuniform = {};

        operator VkDescriptorSet() const { return _descriptorSet; };

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    // ray tracing advanced pipeline
    class Pipeline : public std::enable_shared_from_this<Pipeline> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = {}; // protect from stupid casting

        // 
        VkExtent2D _tiling = { 8u, 8u };

        // 
        std::shared_ptr<Device> _device = {};
        std::shared_ptr<PipelineLayout> _pipelineLayout = {}; // customized pipeline layout, when pipeline was created

        //
        std::vector<VkPipeline> _generationPipeline = {}, _closestHitPipeline = {}, _missHitPipeline = {}, _groupPipelines = {};

        // material and accelerator descriptor sets, that sets to "1" is dedicated by another natives
        std::vector<VkDescriptorSet> _userDefinedDescriptorSets = {}; // beyond than 1 only

        operator VkPipeline() const { return _dullPipeline; };
        
        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    // vertex assembly cache 
    class VertexAssemblySet : public std::enable_shared_from_this<VertexAssemblySet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};

        // vertex and bvh export 
        std::shared_ptr<DeviceImage> _attributeTexelBuffer = {};
        std::shared_ptr<DeviceBuffer> _verticeBuffer = {}, _verticeBufferSide = {}, _verticeBufferIn = {}, _materialBuffer = {}, _bitfieldBuffer = {}, _countersBuffer = {}, _normalBuffer = {};

        // input of vertex source data
        std::vector<std::shared_ptr<VertexInputSet>> _vertexInputs = {};

        // primitive count 
        uint32_t _calculatedPrimitiveCount = 0;

        operator VkDescriptorSet() const { return _descriptorSet; };
        
        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    // vertex assembly program
    class AssemblyPipeline : public std::enable_shared_from_this<AssemblyPipeline> {
    public:
        friend Device;
        VkPipeline _vkPipeline = {}; // protect from stupid casting
        std::weak_ptr<Device> _device = {};
        std::shared_ptr<PipelineLayout> _pipelineLayout = {};
        operator VkPipeline() const { return _vkPipeline; };
    };

    // accelerator store set
    class AcceleratorSet : public std::enable_shared_from_this<AcceleratorSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};
        std::shared_ptr<AcceleratorSetExtensionBase> _EXtension = {};
        std::shared_ptr<VertexAssemblySet> _vertexAssemblySet = {}; // in-bound vertex assembly 
        

        // vertex and bvh export 
        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};
        std::shared_ptr<BufferRegion> _bvhMetaBuffer = {}, _bvhBoxBuffer = {}, _bvhBlockUniform = {};
        uint32_t _entryID = 0, _primitiveCount = -1, _primitiveOffset = 0;
        VtBvhBlock _bvhBlockData = {};

        operator VkDescriptorSet() const { return _descriptorSet; };

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    // ray tracing accelerator structure object
    // planned to merge pipeline programs to device
    class AcceleratorHLBVH2 : public std::enable_shared_from_this<AcceleratorHLBVH2> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = {}; // protect from stupid casting
        std::weak_ptr<Device> _device = {};

        // traverse pipeline
        VkPipeline _intersectionPipeline = {}, _interpolatorPipeline = {};

        // build BVH stages (few stages, in sequences)
        VkPipeline _boxCalcPipeline = {}, _boundingPipeline = {}, _shorthandPipeline = {}, _leafPipeline = {}, /*...radix sort between*/ _buildPipeline = {}, _buildPipelineFirst = {}, _fitPipeline = {}, _leafLinkPipeline = {};

        // static pipeline layout for stages 
        VkPipelineLayout _buildPipelineLayout = {}, _traversePipelineLayout = {};

        // build descriptor set 
        VkDescriptorSet _buildDescriptorSet = {}, _sortDescriptorSet = {};

        // internal buffers
        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};
        std::shared_ptr<BufferRegion> _mortonCodesBuffer = {}, _mortonIndicesBuffer = {}, _leafBuffer = {}, _generalBoundaryResultBuffer = {}, _leafNodeIndices = {}, _currentNodeIndices = {}, _fitStatusBuffer = {}, _countersBuffer = {}, _onWorkBoxes = {};


        operator VkPipeline() const { return _dullPipeline; };
    };


    class RoledBufferBase : public std::enable_shared_from_this<RoledBufferBase> {
    public:
        friend Device;

        VkBuffer _buffer = {};
        std::shared_ptr<Device> _device = {};
        std::shared_ptr<BufferRegion> _bufferRegion = {};

        // direct getters and refers
        VkDescriptorBufferInfo  _descriptorInfo() const;
        VkDescriptorBufferInfo& _descriptorInfo();
        VkBufferView  _bufferView() const;
        VkBufferView& _bufferView();

        // getters and refers attributes
        auto _offset() const { return _descriptorInfo().offset; };
        auto _size() const { return _descriptorInfo().range; };
        auto& _offset() { return _descriptorInfo().offset; };
        auto& _size() { return _descriptorInfo().range; };
        auto _parent() const { return _device; };
        auto& _parent() { return _device; };

        operator VkBufferView&() { return this->_bufferView(); };
        operator VkBufferView() const { return this->_bufferView(); };
        operator VkBuffer&() { return _buffer; }; // cast operator
        operator VkBuffer() const { return _buffer; }; // cast operator

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocation _allocation = {};
        VmaAllocationInfo _allocationInfo = {};
        auto _hostMapped() const { return _allocationInfo.pMappedData; };
#endif

        std::vector<std::shared_ptr<BufferRegion>> _sharedRegions = {};
    };

    // this is wrapped advanced buffer class
    template<VtMemoryUsage U>
    class RoledBuffer : public RoledBufferBase, std::enable_shared_from_this<RoledBuffer<U>> {
    public:
        ~RoledBuffer();

        friend Device;
        friend RoledBufferBase;
    };


    //inline RoledBuffer(){};



    // this is wrapped advanced image class
    class DeviceImage : public std::enable_shared_from_this<DeviceImage> {
    public:
        ~DeviceImage();

        friend Device;
        VkImage _image = {}; VkImageView _imageView = {};
        std::shared_ptr<Device> _device = {};

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocation _allocation = {};
        VmaAllocationInfo _allocationInfo = {};
#endif

        VkImageSubresourceRange _subresourceRange = {};
        VkImageSubresourceLayers _subresourceLayers = {};
        VkImageLayout _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, _layout = VK_IMAGE_LAYOUT_GENERAL;
        VkFormat _format = VK_FORMAT_R32G32B32A32_UINT;
        VkExtent3D _extent = {1u, 1u, 1u};
        VkDescriptorImageInfo _sDescriptorInfo = {};

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };

        operator VkImage() const { return _image; }; // cast operator
        operator VkImage&() { return _image; }; // cast operator
        operator VkImageView() const { return _imageView; }; // cast operator
        operator VkImageView&() { return _imageView; }; // cast operator

        auto  _genDescriptorInfo() const { return VkDescriptorImageInfo{ {}, _imageView, _layout }; };
        auto  _descriptorInfo() const { return this->_sDescriptorInfo; };
        auto& _descriptorInfo() { return (this->_sDescriptorInfo = this->_genDescriptorInfo()); };
    };

    // in-bound buffer region
    class BufferRegion : public std::enable_shared_from_this<BufferRegion> {
    public:
        friend Device;
        VkBufferView _sBufferView = {};
        std::shared_ptr<Device> _device = {};

        std::weak_ptr<RoledBufferBase> _boundBuffer = {};
        VkFormat _format = VK_FORMAT_UNDEFINED; VkDescriptorBufferInfo _sDescriptorInfo = {{}, 0, VK_WHOLE_SIZE};

        auto  _descriptorInfo() const { return _sDescriptorInfo; };
        auto& _descriptorInfo() { return _sDescriptorInfo; };

        auto _offset() const { return _descriptorInfo().offset; };
        auto _size() const { return _descriptorInfo().range; };
        auto& _offset() { return _descriptorInfo().offset; };
        auto& _size() { return _descriptorInfo().range; };
        auto _bufferView() const { return _sBufferView; };
        auto& _bufferView() { return _sBufferView; };
        auto _parent() const { return _device; };
        auto& _parent() { return _device; };

        operator VkDescriptorBufferInfo() const { return _descriptorInfo(); };
        operator VkDescriptorBufferInfo&() { return _descriptorInfo(); };
        
        operator VkBufferView() const { return _bufferView(); };
        operator VkBufferView&() { return _bufferView(); };

        operator VkBuffer() const;
        operator VkBuffer&();
    };


    // avoid compilation issues
     inline BufferRegion::operator VkBuffer&() { return _boundBuffer.lock()->_buffer; };
     inline BufferRegion::operator VkBuffer() const { return _boundBuffer.lock()->_buffer; };
     inline VkDescriptorBufferInfo  RoledBufferBase::_descriptorInfo() const { return _bufferRegion->_descriptorInfo(); };
     inline VkDescriptorBufferInfo& RoledBufferBase::_descriptorInfo() { return _bufferRegion->_descriptorInfo(); };
     inline VkBufferView  RoledBufferBase::_bufferView() const { return _bufferRegion->_bufferView(); };
     inline VkBufferView& RoledBufferBase::_bufferView() { return _bufferRegion->_bufferView(); };



    class BufferManager : public std::enable_shared_from_this<BufferManager> {
    public:
        friend Device;
        std::shared_ptr<Device> _device = {};
        std::shared_ptr<DeviceBuffer> _bufferStore = {};
        std::vector<std::shared_ptr<BufferRegion>> _bufferRegions = {};
        VkDeviceSize _size = 0; // accumulatable size

        // create structuring 
        VtResult _prealloc(VtBufferRegionCreateInfo cinfo, std::shared_ptr<BufferRegion>& bRegion);
    };

    // this class does not using in ray tracing API
    // can be pinned with device
    class RadixSort : public std::enable_shared_from_this<RadixSort> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = {}; // protect from stupid casting
        std::shared_ptr<Device> _device = {};

        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};
        std::shared_ptr<BufferRegion> _histogramBuffer = {};
        std::shared_ptr<BufferRegion> _prefixSumBuffer = {};
        std::shared_ptr<BufferRegion> _stepsBuffer = {}; // constant buffer
        std::shared_ptr<BufferRegion> _tmpKeysBuffer = {}; // cache keys between stages (avoid write conflict)
        std::shared_ptr<BufferRegion> _tmpValuesBuffer = {}; // cache values between stages (avoid write conflict)

        VkPipeline _histogramPipeline = {}, _workPrefixPipeline = {}, _permutePipeline = {}, _copyhackPipeline = {}; // radix sort pipelines
        VkPipelineLayout _pipelineLayout = {}; // use unified pipeline layout 
        VkDescriptorSet _descriptorSet = {};

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
        operator VkPipeline() const { return _dullPipeline; };
    };

    // this class does not using in ray tracing API
    // can be pinned with device 
    // in every copy procedure prefer create own descriptor sets
    // or use push descriptors 
    class CopyProgram : public std::enable_shared_from_this<CopyProgram> {
    public:
        friend Device;
        const VkPipeline _dullPipeline = {}; // protect from stupid casting
        std::shared_ptr<Device> _device = {};

        VkPipeline _bufferCopyPipeline = {}, _bufferCopyIndirectPipeline = {}, _imageCopyPipeline = {}, _imageCopyIndirectPipeline = {};
        VkPipelineLayout _bufferCopyPipelineLayout = {}, _imageCopyPipelineLayout = {};

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
        operator VkPipeline() const { return _dullPipeline; };
    };

    class MaterialSet : public std::enable_shared_from_this<MaterialSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};

        // textures and samplers bound in descriptor set directly

        // material data buffers
        //std::shared_ptr<DeviceBuffer> _virtualSamplerCombinedBuffer;
        //std::shared_ptr<DeviceBuffer> _materialDataBuffer;
        std::shared_ptr<DeviceBuffer> _constBuffer = {};
        uint32_t _materialCount = 0, _materialOffset = 0;

        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };

    class VertexInputSet : public std::enable_shared_from_this<VertexInputSet> {
    public:
        friend Device;
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};
        VtUniformBlock _uniformBlock = {};

        // vertex assembly pipeline bound
        std::shared_ptr<DeviceBuffer> _uniformBlockBuffer = {}; // binding of uniform arrays
        std::shared_ptr<AssemblyPipeline> _attributeVertexAssembly = {}; //
        std::shared_ptr<DeviceBuffer> _inlineTransformBuffer = {}; // if have no required

        // TODO: RTX capable buffers 
        

        auto  _parent() const { return _device; };
        auto& _parent() { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };

        auto& uniform() { return _uniformBlock; };
         auto uniform() const { return _uniformBlock; };
    };




    // 
    class AdvancedAcceleratorDataBase : public std::enable_shared_from_this<AdvancedAcceleratorDataBase> {
    public:
        friend Device;
        virtual ~AdvancedAcceleratorDataBase() {}; // with overridabable virtual desctructor
    };

    // 
    class AcceleratorSetExtensionDataBase : public std::enable_shared_from_this<AcceleratorSetExtensionDataBase> {
    public:
        friend Device;
        virtual ~AcceleratorSetExtensionDataBase() {}; // with overridabable virtual desctructor
    };

    // unused, planned in 2019 
    class AdvancedAcceleratorBase : public std::enable_shared_from_this<AdvancedAcceleratorBase> {
    public:
        friend Device;

    protected:
        std::weak_ptr<Device> _device = {};
        std::shared_ptr<AdvancedAcceleratorDataBase> _dataPtr = {};

    public:
        virtual VtAccelerationName _AccelerationName() const { return VT_ACCELERAION_NAME_UNKNOWN; };

        // minimal requirement of... 
        virtual VtResult _DoIntersections(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, std::shared_ptr<RayTracingSet> rayTracingSet) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        };
        virtual VtResult _BuildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        };
        virtual VtResult _Init(const VtDeviceAdvancedAccelerationExtension* extensionStructure = nullptr) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }; // initialize by extension 

         // built-in method's
        auto* operator->() { return _dataPtr.get(); };
        auto* operator->() const { return _dataPtr.get(); };
    };

    // planned in 2019
    class AcceleratorSetExtensionBase : public std::enable_shared_from_this<AcceleratorSetExtensionBase> {
    public:
        friend Device;

    protected:
        std::weak_ptr<Device> _device = {};
        std::shared_ptr<AcceleratorSetExtensionDataBase> _dataPtr = {};

    public:
        //operator VkDescriptorSet() const { return _descriptorSet; };
        virtual VtAccelerationName _AccelerationName() const { return VT_ACCELERAION_NAME_UNKNOWN; };
        virtual VtResult _Init(const void* extensionStructure = nullptr) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }; // initialize by extension (TODO)
        virtual VtResult _Construction(std::shared_ptr<AcceleratorSet> accelSet = {}) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }; // accessing by same address
        
        auto  _parent() const { return _device; };
        auto& _parent() { return _device; };

        // built-in method's
        auto* operator->() { return _dataPtr.get(); };
        auto* operator->() const { return _dataPtr.get(); };
    };

};
