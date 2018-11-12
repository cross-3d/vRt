#pragma once

// implementable weak classes
#include "HardClassesDef.inl"
#include "../../Parts/StructuresDef.inl"

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
        //std::shared_ptr<HostToDeviceBuffer> _uploadBuffer = {}; // from host
        //std::shared_ptr<DeviceToHostBuffer> _downloadBuffer = {}; // to host
        std::shared_ptr<DeviceBuffer>       _uniformVIBuffer = {};
        std::shared_ptr<HostToDeviceBuffer> _uniformVIMapped = {};
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
        std::vector<VkExtensionProperties> _extensions = {};
        VkPhysicalDeviceProperties2 _properties = {};
        VkPhysicalDevice16BitStorageFeatures _storage16 = {};
        VkPhysicalDevice8BitStorageFeaturesKHR _storage8 = {};
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT _descriptorIndexing = {};

#ifdef VT_LEGACY_RAYTRACING_NVX
        VkPhysicalDeviceRaytracingPropertiesNVX _rayTracingNV = {};
#else
        VkPhysicalDeviceRayTracingPropertiesNV _rayTracingNV = {};
#endif

        // features linking
        operator VkPhysicalDeviceFeatures2&() { return _features; };
        operator VkPhysicalDeviceFeatures2() const { return _features; };
        operator VkPhysicalDeviceProperties2&() { return _properties; };
        operator VkPhysicalDeviceProperties2() const { return _properties; };

        auto _parent() const { return _physicalDevice; };
        auto& _parent() { return _physicalDevice; };
    };

    // ray tracing device with aggregation
    class Device : public std::enable_shared_from_this<Device> {
    public:
        ~Device();

        friend PhysicalDevice;
        VkDevice _device = {};
        std::shared_ptr<DeviceFeatures> _features = {};
        std::shared_ptr<PhysicalDevice> _physicalDevice = {};

        //uint32_t _mainFamilyIndex = 0;
        std::vector<uint32_t> _familyIndices = {};
        std::string _shadersPath = "./intrusive";

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocator _allocator = {};
        operator VmaAllocator() const { return _allocator; };
        operator VmaAllocator&() { return _allocator; };
#endif

        VkPipelineCache _pipelineCache = {}; // store native pipeline cache
        VkDescriptorPool _descriptorPool = {};
        VkDescriptorSet _emptyDS = {};
        VkShaderStageFlags _descriptorAccess = VK_SHADER_STAGE_COMPUTE_BIT;

        // TODO: optional when enabled extensions 
        uint32_t _supportedThreadCount = 1u;
        std::vector<std::shared_ptr<RadixSort>> _radixSort = {}; // 
        std::vector<std::shared_ptr<AcceleratorHLBVH2>> _acceleratorBuilder = {}; // planned to rename
        std::vector<std::shared_ptr<AssemblyPipeline>> _nativeVertexAssembler = {};
        //std::vector<std::shared_ptr<BufferTraffic>> _bufferTraffic = {};

        // accelerator by extension
        std::vector<std::shared_ptr<AcceleratorExtensionBase>> _hExtensionAccelerator = {};

        // descriptor layout map in ray tracing system
        std::map<std::string, VkDescriptorSetLayout> _descriptorLayoutMap = {};
        VtVendor _vendorName = VT_VENDOR_UNIVERSAL;

        operator VkDevice() const { return _device; };
        operator VkPipelineCache() const { return _pipelineCache; };
        operator VkDescriptorPool() const { return _descriptorPool; };

        //operator std::shared_ptr<HostToDeviceBuffer>() const { return _bufferTraffic[0]->_uploadBuffer; };
        //operator std::shared_ptr<DeviceToHostBuffer>() const { return _bufferTraffic[0]->_downloadBuffer; };

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
        VkPipelineLayout _vsLayout = {}, _rtLayout = {}; // replaced set 0 and 1
        VtPipelineLayoutType _type = VT_PIPELINE_LAYOUT_TYPE_RAYTRACING;
        std::shared_ptr<Device> _device = {};

        operator VkPipelineLayout() const { return _vsLayout; }; // by default should be return vertex version
        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    
    class RayTracingSet : public std::enable_shared_from_this<RayTracingSet> {
    public:
        friend Device;
        ~RayTracingSet();
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
            _taskBuffer = {},
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
        std::vector<VkPipeline> _generationPipeline = {}, _closestHitPipeline = {}, _missHitPipeline = {}, _groupPipeline = {};

        // material and accelerator descriptor sets, that sets to "1" is dedicated by another natives
        std::vector<VkDescriptorSet> _userDefinedDescriptorSets = {}; // beyond than 1 only

        operator VkPipeline() const { return _dullPipeline; };
        
        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
    };

    // 
    // attribute and vertex space productor for single structured instance 
    // for correct instanced linking require shared buffer for data containing 
    class VertexAssemblySet : public std::enable_shared_from_this<VertexAssemblySet> {
    public:
        friend Device;
        ~VertexAssemblySet();
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};
        std::shared_ptr<VertexAssemblyExtensionBase> _hExtension = {};

        // DEPRECATED, planned to fully remove, since need merge to dedicated descriptor set 
        std::shared_ptr<DeviceImage> _attributeTexelBuffer = {};
        std::shared_ptr<AssemblyPipeline> _vertexAssembly = {}; //

        // vertex for BVH export
        std::shared_ptr<BufferRegion> _verticeBufferCached = {}, _verticeBufferInUse = {}, _materialBuffer = {}, _bitfieldBuffer = {}, _countersBuffer = {}, _normalBuffer = {}, _indexBuffer = {};
        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};

        // input of vertex source data 
        std::vector<std::shared_ptr<BufferTraffic>> _bufferTraffic = {};
        std::vector<std::shared_ptr<VertexInputSet>> _vertexInputs = {};
        std::function<void()> _descriptorSetGenerator = {};

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
        VkPipeline _inputPipeline = {}, _intrpPipeline = {};
        std::weak_ptr<Device> _device = {};
        std::shared_ptr<PipelineLayout> _pipelineLayout = {};
        operator VkPipeline() const { return _inputPipeline; };
    };



    // accelerator store set
    class AcceleratorSet : public std::enable_shared_from_this<AcceleratorSet> {
    public:
        friend Device;
        ~AcceleratorSet();
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};
        std::shared_ptr<AcceleratorSetExtensionBase> _hExtension = {};

        std::shared_ptr<VertexAssemblySet> _vertexAssemblySet = {}; // in-bound vertex assembly
        std::vector<std::shared_ptr<AcceleratorSet>> _usedAcceleratorSets = {};

        // vertex and bvh export 
        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};
        std::shared_ptr<BufferRegion> _bvhBoxBuffer = {}, _bvhHeadingBuffer = {}; // 
        std::shared_ptr<BufferRegion> _bvhHeadingInBuffer = {}, _bvhInstancedBuffer = {}, _bvhTransformBuffer = {}; // shared buffer with multiple BVH data 
        std::function<void()> _descriptorSetGenerator = {};

        // planned to rework building system  
        VtAcceleratorSetLevel _level = VT_ACCELERATOR_SET_LEVEL_GEOMETRY;
        VkDeviceSize _entryID = 0, _elementsCount = -1, _elementsOffset = 0;
        VtMat4 _coverMatrice = IdentifyMat4;
        VtBvhBlock _bvhBlockData = {};
        VkDeviceSize _capacity = 0ull;

        operator VkDescriptorSet() const { return _descriptorSet; };
        auto  _parent() const { return _device; };
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
        VkPipeline _intersectionPipeline = {};

        // build BVH stages (few stages, in sequences)
        VkPipeline _boundingPipeline = {}, _shorthandPipeline = {}, /*...radix sort between*/ _buildPipeline = {}, _buildPipelineFirst = {}, _fitPipeline = {}, _leafLinkPipeline = {};
        std::vector<VkPipeline> _boxCalcPipeline = {}, _leafPipeline = {};

        // static pipeline layout for stages 
        VkPipelineLayout _buildPipelineLayout = {}, _traversePipelineLayout = {};

        // build descriptor set 
        VkDescriptorSet _buildDescriptorSet = {}, _sortDescriptorSet = {};

        // internal buffers
        std::shared_ptr<DeviceBuffer> _sharedBuffer = {};
        std::shared_ptr<BufferRegion> _mortonCodesBuffer = {}, _mortonIndicesBuffer = {}, _leafBuffer = {}, _generalBoundaryResultBuffer = {}, _leafNodeIndices = {}, _currentNodeIndices = {}, _fitStatusBuffer = {}, _countersBuffer = {}, _onWorkBoxes = {}, _constBuffer = {};

        // 
        VtBuildConst _buildConstData = {};

        operator VkPipeline() const { return _dullPipeline; };
    };


    class RoledBufferBase : public std::enable_shared_from_this<RoledBufferBase> {
    public:
        friend Device;
        ~RoledBufferBase();

        VkBuffer _buffer = {};
        std::shared_ptr<Device> _device = {};

        operator VkBuffer&() { return _buffer; }; // cast operator
        operator VkBuffer() const { return _buffer; }; // cast operator

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        VmaAllocation _allocation = {};
        VmaAllocationInfo _allocationInfo = {};
        auto _hostMapped() const { return _allocationInfo.pMappedData; };
#endif
    };

    // this is wrapped advanced buffer class
    template<VtMemoryUsage U>
    //class RoledBuffer : public RoledBufferBase, std::enable_shared_from_this<RoledBuffer<U>> {
    class RoledBuffer : public std::enable_shared_from_this<RoledBuffer<U>> {
    public:
        friend Device;
        friend RoledBufferBase;

        //~RoledBuffer();
        std::shared_ptr<RoledBufferBase> _bufferWrap = {}; // 
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
        auto _parent() const { return _device(); };
        auto& _parent() { return _device(); };

        operator std::shared_ptr<RoledBufferBase>&() { return _bufferWrap; };
        operator std::shared_ptr<RoledBufferBase>() const { return _bufferWrap; };

        // typed getters and refers
        operator VkBufferView&() { return this->_bufferView(); };
        operator VkBufferView() const { return this->_bufferView(); };
        operator VkBuffer&() { return _bufferWrap->_buffer; }; // cast operator
        operator VkBuffer() const { return _bufferWrap->_buffer; }; // cast operator
        auto _hostMapped() const { return _bufferWrap->_hostMapped(); };

        // direct getters and refers
        VkDevice  _device() const { return *_bufferWrap->_device; };
        //VkDevice& _device() { return *_bufferWrap->_device; };
        VkBuffer  _buffer() const { return _bufferWrap->_buffer; };
        VkBuffer& _buffer() { return _bufferWrap->_buffer; };

        // redirectors
        //VkDescriptorBufferInfo  _descriptorInfo() const { return _bufferWrap->_descriptorInfo(); };
        //VkDescriptorBufferInfo& _descriptorInfo() { return _bufferWrap->_descriptorInfo(); };
        //VkBufferView  _bufferView() const { return _bufferWrap->_bufferView(); };
        //VkBufferView& _bufferView() { return _bufferWrap->_bufferView(); };
        //std::shared_ptr<BufferRegion>  _bufferRegion() const { return _bufferWrap->_bufferRegion; };
        //std::shared_ptr<BufferRegion>& _bufferRegion() { return _bufferWrap->_bufferRegion; };
        //std::vector<std::shared_ptr<BufferRegion>>  _sharedRegions() const { return _bufferWrap->_sharedRegions; };
        //std::vector<std::shared_ptr<BufferRegion>>& _sharedRegions() { return _bufferWrap->_sharedRegions; };

        // 
        std::vector<std::shared_ptr<BufferRegion>> _sharedRegions = {};
    };


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
        ~BufferRegion();
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
     inline BufferRegion::operator VkBuffer&() { return _descriptorInfo().buffer; };
     inline BufferRegion::operator VkBuffer() const { return _descriptorInfo().buffer; };
     template<VtMemoryUsage U> inline VkDescriptorBufferInfo  RoledBuffer<U>::_descriptorInfo() const { return _bufferRegion->_descriptorInfo(); };
     template<VtMemoryUsage U> inline VkDescriptorBufferInfo& RoledBuffer<U>::_descriptorInfo() { return _bufferRegion->_descriptorInfo(); };
     template<VtMemoryUsage U> inline VkBufferView  RoledBuffer<U>::_bufferView() const { return _bufferRegion->_bufferView(); };
     template<VtMemoryUsage U> inline VkBufferView& RoledBuffer<U>::_bufferView() { return _bufferRegion->_bufferView(); };



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


    // may to be deprecated due over-roling 
    class MaterialSet : public std::enable_shared_from_this<MaterialSet> {
    public:
        friend Device;
        ~MaterialSet();
        VkDescriptorSet _descriptorSet = {}; // textures and samplers bound in descriptor set directly
        std::shared_ptr<Device> _device = {};

        // material data buffers
        std::shared_ptr<DeviceBuffer> _constBuffer = {};
        uint32_t _materialCount = 0, _materialOffset = 0;


        auto _parent() const { return _device; };
        auto& _parent() { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };
    };


    // 
    class VertexInputSet : public std::enable_shared_from_this<VertexInputSet> {
    public:
        friend Device;
        ~VertexInputSet();
        VkDescriptorSet _descriptorSet = {};
        std::shared_ptr<Device> _device = {};
        VtUniformBlock _uniformBlock = {};

        // vertex assembly pipeline bound
        std::shared_ptr<DeviceBuffer> _uniformBlockBuffer = {}; // binding of uniform arrays
        std::shared_ptr<AssemblyPipeline> _attributeAssembly = {}; //
        std::shared_ptr<DeviceBuffer> _inlineTransformBuffer = {}; // if have no required
        std::function<void()> _descriptorSetGenerator = {};

        auto  _parent() const { return _device; };
        auto& _parent() { return _device; };
        operator VkDescriptorSet() const { return _descriptorSet; };

        auto  uniform() const { return _uniformBlock; };
        auto& uniform() { return _uniformBlock; };
    };



    // 
    class AcceleratorExtensionBase : public std::enable_shared_from_this<AcceleratorExtensionBase> {
    public:
        friend Device;

    protected:
        VtAccelerationName _nativeAccelerationName = VT_ACCELERATION_NAME_UNKNOWN;
        std::weak_ptr<Device> _device = {};
        //std::shared_ptr<AdvancedAcceleratorDataBase> _dataPtr = {};

    public:
        virtual VtAccelerationName _AccelerationName() const { return VT_ACCELERATION_NAME_UNKNOWN; };

        // 
        virtual VtResult _DoIntersections(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, std::shared_ptr<RayTracingSet> rayTracingSet);
        virtual VtResult _BuildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet> acceleratorSet, VtAcceleratorBuildInfo buildInfo);
        virtual VtResult _Init(std::shared_ptr<Device> device, const VtDeviceAdvancedAccelerationExtension * extensionBasedInfo);
        virtual VtResult _Criteria(std::shared_ptr<DeviceFeatures> supportedFeatures);

        // connectors with extension classes
        virtual VtResult _ConstructAcceleratorSet(std::shared_ptr<AcceleratorSet> accelSet = {});
        virtual VtResult _ConstructVertexAssembly(std::shared_ptr<VertexAssemblySet> assemblySet = {});

         // built-in method's
        //auto* operator->() { return _dataPtr.get(); };
        //auto* operator->() const { return _dataPtr.get(); };
    };

    // 
    class AcceleratorSetExtensionBase : public std::enable_shared_from_this<AcceleratorSetExtensionBase> {
    public:
        friend Device;

    protected:
        VtAccelerationName _nativeAccelerationName = VT_ACCELERATION_NAME_UNKNOWN;
        std::weak_ptr<AcceleratorSet> _accelSet = {};
        std::weak_ptr<Device> _device = {};
        
        //std::shared_ptr<AcceleratorSetExtensionDataBase> _dataPtr = {};

    public:
        //operator VkDescriptorSet() const { return _descriptorSet; };
        virtual VtAccelerationName _AccelerationName() const { return VT_ACCELERATION_NAME_UNKNOWN; };

        virtual VtResult _Construction(std::shared_ptr<AcceleratorSet> accelSet = {}) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }; // accessing by same address
        
        auto  _parent() const { return _device; };
        auto& _parent() { return _device; };

        // built-in method's
        //auto* operator->() { return _dataPtr.get(); };
        //auto* operator->() const { return _dataPtr.get(); };
    };

    // 
    class VertexAssemblyExtensionBase : public std::enable_shared_from_this<VertexAssemblyExtensionBase> {
    public:
        friend Device;

    protected:
        VtAccelerationName _nativeAccelerationName = VT_ACCELERATION_NAME_UNKNOWN;
        std::weak_ptr<VertexAssemblySet> _assemblySet = {};
        std::weak_ptr<Device> _device = {};
        
        //std::shared_ptr<VertexAssemblyExtensionDataBase> _dataPtr = {};

    public:
        //operator VkDescriptorSet() const { return _descriptorSet; };
        virtual VtAccelerationName _AccelerationName() const { return VT_ACCELERATION_NAME_UNKNOWN; };

        virtual VtResult _Construction(std::shared_ptr<VertexAssemblySet> accelSet = {}) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }; // accessing by same address

        auto  _parent() const { return _device; };
        auto& _parent() { return _device; };

        // built-in method's
        //auto* operator->() { return _dataPtr.get(); };
        //auto* operator->() const { return _dataPtr.get(); };
    };



};
