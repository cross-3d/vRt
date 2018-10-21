#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;


    Device::~Device() {
        // HANGING ON RX VEGA GPU 
        //if (_device) vk::Device(_device).waitIdle(); // just wait device idle for destructor 
        _device = {};
    };


    VtResult convertDevice(VkDevice device, std::shared_ptr<PhysicalDevice> physicalDevice, VtArtificalDeviceExtension vtExtension, std::shared_ptr<Device>& vtDevice) {
        //auto vtDevice = (_vtDevice = std::make_shared<Device>());
        vtDevice = std::make_shared<Device>();
        vtDevice->_physicalDevice = physicalDevice; // reference for aliasing
        auto gpu = vk::PhysicalDevice(physicalDevice->_physicalDevice);

        // minimal features
        auto features = (vtDevice->_features = std::make_shared<DeviceFeatures>());
        features->_features = vk::PhysicalDeviceFeatures2{};
        features->_storage8 = vk::PhysicalDevice8BitStorageFeaturesKHR{};
        features->_storage16 = vk::PhysicalDevice16BitStorageFeatures{};
        features->_descriptorIndexing = vk::PhysicalDeviceDescriptorIndexingFeaturesEXT{};
        features->_features.pNext = &features->_storage16, features->_storage16.pNext = &features->_storage8, features->_storage8.pNext = &features->_descriptorIndexing;
        gpu.getFeatures2((vk::PhysicalDeviceFeatures2*)&features->_features); // get features support by physical devices

        // device vendoring
        vtDevice->_device = device;
        vtDevice->_vendorName = getVendorName(gpu.getProperties().vendorID);


        VtResult result = VK_SUCCESS;
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        if (vtExtension.allocator) {
            vtDevice->_allocator = *(const VmaAllocator*)vtExtension.allocator; result = VK_SUCCESS;
        } else {
            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.physicalDevice = *(vtDevice->_physicalDevice);
            allocatorInfo.device = vtDevice->_device;
            allocatorInfo.preferredLargeHeapBlockSize = 32 * sizeof(uint32_t);
            allocatorInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            allocatorInfo.pAllocationCallbacks = nullptr;
            allocatorInfo.pVulkanFunctions = nullptr;
            allocatorInfo.pHeapSizeLimit = nullptr;

#ifdef VOLK_H_
            // load API calls for mapping
            VolkDeviceTable vktable = {};
            volkLoadDeviceTable(&vktable, vtDevice->_device);

            // mapping volk with VMA functions
            VmaVulkanFunctions vfuncs = {};
            vfuncs.vkAllocateMemory = vktable.vkAllocateMemory;
            vfuncs.vkBindBufferMemory = vktable.vkBindBufferMemory;
            vfuncs.vkBindImageMemory = vktable.vkBindImageMemory;
            vfuncs.vkCreateBuffer = vktable.vkCreateBuffer;
            vfuncs.vkCreateImage = vktable.vkCreateImage;
            vfuncs.vkDestroyBuffer = vktable.vkDestroyBuffer;
            vfuncs.vkDestroyImage = vktable.vkDestroyImage;
            vfuncs.vkFreeMemory = vktable.vkFreeMemory;
            vfuncs.vkGetBufferMemoryRequirements = vktable.vkGetBufferMemoryRequirements;
            vfuncs.vkGetBufferMemoryRequirements2KHR = vktable.vkGetBufferMemoryRequirements2KHR;
            vfuncs.vkGetImageMemoryRequirements = vktable.vkGetImageMemoryRequirements;
            vfuncs.vkGetImageMemoryRequirements2KHR = vktable.vkGetImageMemoryRequirements2KHR;
            vfuncs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
            vfuncs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
            vfuncs.vkMapMemory = vktable.vkMapMemory;
            vfuncs.vkUnmapMemory = vktable.vkUnmapMemory;
            allocatorInfo.pVulkanFunctions = &vfuncs;
#endif

            if (vmaCreateAllocator(&allocatorInfo, &vtDevice->_allocator) == VK_SUCCESS) { result = VK_SUCCESS; };
        }
#endif

        // link device with vulkan.hpp
        auto _device = vk::Device(vtDevice->_device);

        // create default pipeline cache
        auto vkPipelineCache = VkPipelineCache(_device.createPipelineCache(vk::PipelineCacheCreateInfo()));
        vtDevice->_pipelineCache = vkPipelineCache;

        // make descriptor pool
        const size_t mult = 4;
        std::vector<vk::DescriptorPoolSize> dps = { 
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eSampledImage).setDescriptorCount(256 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eStorageImage).setDescriptorCount(256 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eStorageBuffer).setDescriptorCount(256 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(256 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eSampler).setDescriptorCount(64 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(256 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eStorageTexelBuffer).setDescriptorCount(256 * mult),
            vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformTexelBuffer).setDescriptorCount(256 * mult),
        };

        vtDevice->_descriptorPool = VkDescriptorPool(_device.createDescriptorPool(vk::DescriptorPoolCreateInfo().setMaxSets(1024).setPPoolSizes(dps.data()).setPoolSizeCount(dps.size()).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT)));
        vtDevice->_mainFamilyIndex = vtExtension.mainQueueFamily;
        vtDevice->_shadersPath = vtExtension.shaderPath;




        // buffer <--> host traffic buffers
        { const constexpr uint32_t t = 0u;
            vtDevice->_bufferTraffic.push_back(std::make_shared<BufferTraffic>());

            // make traffic buffers 
            VtDeviceBufferCreateInfo dbfi = {};
            dbfi.bufferSize = strided<uint32_t>(vtExtension.sharedCacheSize);
            dbfi.format = VkFormat(vk::Format::eR8Uint); // just uint8_t data
            dbfi.familyIndex = vtExtension.mainQueueFamily;

            vtDevice->_bufferTraffic[t]->_device = vtDevice;
            createHostToDeviceBuffer(vtDevice, dbfi, vtDevice->_bufferTraffic[t]->_uploadBuffer);
            createDeviceToHostBuffer(vtDevice, dbfi, vtDevice->_bufferTraffic[t]->_downloadBuffer);
        };

        // vertex input meta construction arrays
        { const constexpr uint32_t t = 0u;
            VtDeviceBufferCreateInfo dbfi = {};
            dbfi.format = VK_FORMAT_UNDEFINED;
            dbfi.bufferSize = strided<VtUniformBlock>(1024);
            dbfi.familyIndex = vtExtension.mainQueueFamily;
            createDeviceBuffer(vtDevice, dbfi, vtDevice->_bufferTraffic[t]->_uniformVIBuffer);
        };




        //
        auto pbindings = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind | vk::DescriptorBindingFlagBitsEXT::eVariableDescriptorCount | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;
        auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(&pbindings);
        auto vkpi = vk::DescriptorSetLayoutCreateInfo().setPNext(&vkfl);

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // rays
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // hit heads
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // closest hit indices
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // miss hit indices 
                vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // hit payloads
                vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray indices 
                vk::DescriptorSetLayoutBinding(6, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // constant buffer
                vk::DescriptorSetLayoutBinding(7, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // counters 
                vk::DescriptorSetLayoutBinding(8, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),  // task lists
                vk::DescriptorSetLayoutBinding(9, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
                vk::DescriptorSetLayoutBinding(10, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray<->hit binding payload 
                vk::DescriptorSetLayoutBinding(11, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute),
                vk::DescriptorSetLayoutBinding(12, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // group counters 
                vk::DescriptorSetLayoutBinding(13, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // group counters for reads
                vk::DescriptorSetLayoutBinding(14, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray indices for reads
            };
            //vkfl.setBindingCount(_bindings.size());
            vtDevice->_descriptorLayoutMap["rayTracing"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh main block 
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh nodes 
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh instances
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh blocks  
            };
            vtDevice->_descriptorLayoutMap["hlbvh2"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // morton codes
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // morton indices
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // const buffer 
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // leafs
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // aabb on work
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vote flags
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process indices
                vk::DescriptorSetLayoutBinding(7 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // leaf node indices
                vk::DescriptorSetLayoutBinding(8 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process counters
                vk::DescriptorSetLayoutBinding(9 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // box calculation of scene 
            };
            vtDevice->_descriptorLayoutMap["hlbvh2work"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vertex assembly counters
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material buffer (unused)
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // order buffer (unused)
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // writable vertices
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute), // writable attributes (DEPRECATED row)
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // readonly vertices
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // readonly attributes (DEPRECATED row)
                vk::DescriptorSetLayoutBinding(7 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // precomputed normals
            };
            vtDevice->_descriptorLayoutMap["vertexData"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // keys in
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values in
            };
            vtDevice->_descriptorLayoutMap["radixSortBind"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // keys cache
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values cache
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // radice step properties
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // histogram of radices (every work group)
                vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // prefix-sum of radices (every work group)
            };
            vtDevice->_descriptorLayoutMap["radixSort"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eSampledImage, VRT_MAX_IMAGES, vk::ShaderStageFlagBits::eCompute), // textures
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eSampler, VRT_MAX_SAMPLERS, vk::ShaderStageFlagBits::eCompute), // samplers
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material buffer
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // virtual texture and sampler combinations
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material set uniform 
            };
            vtDevice->_descriptorLayoutMap["materialSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                //vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eUniformTexelBuffer, vendorName == VT_VENDOR_INTEL ? 1 : 8, vk::ShaderStageFlagBits::eCompute), // vertex raw data
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eUniformTexelBuffer, 8, vk::ShaderStageFlagBits::eCompute), // vertex raw data
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // virtual regions
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // buffer views
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // accessors
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // attribute bindings 
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vertex input uniform
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
            };
            vtDevice->_descriptorLayoutMap["vertexInputSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        // 
        const auto vendorName = vtDevice->_vendorName;
        auto simfo = VtAttributePipelineCreateInfo{};
//#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
//        simfo.assemblyModule = makeComputePipelineStageInfo(*vtDevice, natives::vertexAssembly.at(vendorName));
//#else
        simfo.assemblyModule = makeComputePipelineStageInfo(*vtDevice, _vt::readBinary(natives::vertexAssembly.at(vendorName)));
        simfo.interpolModule = makeComputePipelineStageInfo(*vtDevice, _vt::readBinary(hlbvh2::interpolator.at(vendorName)));
//#endif

        // native vertex input pipeline layout 
        auto vtpl = VtPipelineLayoutCreateInfo{};
        createPipelineLayout(vtDevice, vtpl, simfo.pipelineLayout, VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT);

        // create native tools 
        for ( uint32_t t = 0; t < vtDevice->_supportedThreadCount; t++ ) {
            vtDevice->_radixSort.push_back({});             createRadixSort(vtDevice, vtExtension, vtDevice->_radixSort[t]);
            vtDevice->_nativeVertexAssembler.push_back({}); createAssemblyPipeline(vtDevice, simfo, vtDevice->_nativeVertexAssembler[t], true);
            vtDevice->_acceleratorBuilder.push_back({});    createAcceleratorHLBVH2(vtDevice, vtExtension, vtDevice->_acceleratorBuilder[t]);
        }

        // create dull barrier pipeline
        auto rng = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2));
        auto ppl = vk::Device(device).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, 0, nullptr, 0, nullptr));
        //vtDevice->_dullBarrier = createComputeHC(device, natives::dullBarrier.at(vtDevice->_vendorName), ppl, vkPipelineCache);

        return result;
    };


    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo vdvi, std::shared_ptr<Device>& _vtDevice) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        // default structure values
        VtArtificalDeviceExtension vtExtension = {};
        auto vtExtensionPtr = vtSearchStructure(vdvi, VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION);

        // if found, getting some info
        if (vtExtensionPtr) vtExtension = (VtArtificalDeviceExtension&)*vtExtensionPtr;

        // be occurate with "VkDeviceCreateInfo", because after creation device, all "vt" extended structures will destoyed
        VkDevice vkDevice = {};
        if (vkCreateDevice(*physicalDevice, (const VkDeviceCreateInfo*)vtExplodeArtificals(vdvi), nullptr, &vkDevice) == VK_SUCCESS) { result = VK_SUCCESS; };

        // manually convert device
        convertDevice(vkDevice, physicalDevice, vtExtension, _vtDevice);
        return result;
    };


};
