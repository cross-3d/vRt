#pragma once

#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"


namespace _vt {
    using namespace vrt;


    VtResult convertDevice(VkDevice device, std::shared_ptr<PhysicalDevice> physicalDevice, const VtArtificalDeviceExtension& vtExtension, std::shared_ptr<Device>& vtDevice) {
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
            vtDevice->_allocator = vtExtension.allocator; result = VK_SUCCESS;
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
            VolkDeviceTable vktable;
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

        vtDevice->_descriptorPool = VkDescriptorPool(_device.createDescriptorPool(vk::DescriptorPoolCreateInfo().setMaxSets(1024).setPPoolSizes(dps.data()).setPoolSizeCount(dps.size())));
        vtDevice->_mainFamilyIndex = vtExtension.mainQueueFamily;
        vtDevice->_shadersPath = vtExtension.shaderPath;

        // make traffic buffers 
        VtDeviceBufferCreateInfo dbfi;
        dbfi.bufferSize = strided<uint32_t>(vtExtension.sharedCacheSize);
        dbfi.format = VkFormat(vk::Format::eR8Uint); // just uint8_t data
        dbfi.familyIndex = vtExtension.mainQueueFamily;

        // make weak proxy (avoid cycled linking)
        vtDevice->_bufferTraffic = std::make_shared<BufferTraffic>();
        vtDevice->_bufferTraffic->_device = vtDevice;
        createHostToDeviceBuffer(vtDevice, dbfi, vtDevice->_bufferTraffic->_uploadBuffer);
        createDeviceToHostBuffer(vtDevice, dbfi, vtDevice->_bufferTraffic->_downloadBuffer);

        dbfi.format = VK_FORMAT_UNDEFINED;
        dbfi.bufferSize = strided<VtUniformBlock>(1024);
        createDeviceBuffer(vtDevice, dbfi, vtDevice->_bufferTraffic->_uniformVIBuffer);
        const auto vendorName = vtDevice->_vendorName;

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
                vk::DescriptorSetLayoutBinding(8, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // 9-line 
                vk::DescriptorSetLayoutBinding(9, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
                vk::DescriptorSetLayoutBinding(10, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray<->hit binding payload 
                vk::DescriptorSetLayoutBinding(11, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute),
                vk::DescriptorSetLayoutBinding(12, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // group counters 
                vk::DescriptorSetLayoutBinding(13, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // group counters for reads
                vk::DescriptorSetLayoutBinding(14, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray indices for reads
            };
            vtDevice->_descriptorLayoutMap["rayTracing"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh uniform block
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh meta 
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // box boxes 
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh meta (bufferView)
            };
            vtDevice->_descriptorLayoutMap["hlbvh2"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // morton codes
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // morton indices

                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // leafs
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // aabb on work
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vote flags
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process indices
                vk::DescriptorSetLayoutBinding(7 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // leaf node indices
                vk::DescriptorSetLayoutBinding(8 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process counters
                vk::DescriptorSetLayoutBinding(9 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process scene box
            };
            vtDevice->_descriptorLayoutMap["hlbvh2work"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vertex assembly counters
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material buffer (unused)
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // order buffer (unused)
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // writable vertices
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute), // writable attributes
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // readonly vertices
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // readonly attributes
                vk::DescriptorSetLayoutBinding(7 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // precomputed normals
            };
            vtDevice->_descriptorLayoutMap["vertexData"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // keys in
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values in
            };
            vtDevice->_descriptorLayoutMap["radixSortBind"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // keys cache
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values cache
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // radice step properties
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // histogram of radices (every work group)
                vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // prefix-sum of radices (every work group)
            };
            vtDevice->_descriptorLayoutMap["radixSort"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eSampledImage, VRT_MAX_IMAGES, vk::ShaderStageFlagBits::eCompute), // textures
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eSampler, VRT_MAX_SAMPLERS, vk::ShaderStageFlagBits::eCompute), // samplers
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material buffer
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // virtual texture and sampler combinations
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material set uniform 
            };
            vtDevice->_descriptorLayoutMap["materialSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

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
            vtDevice->_descriptorLayoutMap["vertexInputSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        // 
        auto simfo = VtVertexAssemblyPipelineCreateInfo{};
#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
        simfo.vertexAssemblyModule = makeComputePipelineStageInfo(*vtDevice, natives::vertexAssembly.at(vendorName));
#else
        simfo.vertexAssemblyModule = makeComputePipelineStageInfo(*vtDevice, _vt::readBinary(natives::vertexAssembly.at(vendorName)));
#endif

        auto vtpl = VtPipelineLayoutCreateInfo{};
        createPipelineLayout(vtDevice, vtpl, simfo.pipelineLayout, VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT);

        // create radix sort tool
        createRadixSort(vtDevice, vtExtension, vtDevice->_radixSort);
        createVertexAssemblyPipeline(vtDevice, simfo, vtDevice->_vertexAssembler);
        createAcceleratorHLBVH2(vtDevice, vtExtension, vtDevice->_acceleratorBuilder);

        // create dull barrier pipeline
        auto rng = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2));
        auto ppl = vk::Device(device).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, 0, nullptr, 0, nullptr));
        //vtDevice->_dullBarrier = createComputeHC(device, natives::dullBarrier.at(vtDevice->_vendorName), ppl, vkPipelineCache);

        return result;
    };


    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo vdvi, std::shared_ptr<Device>& _vtDevice) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;
        VtArtificalDeviceExtension vtExtension; // default structure values
        auto vtExtensionPtr = vtSearchStructure(vdvi, VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION);
        if (vtExtensionPtr) { // if found, getting some info
            vtExtension = (VtArtificalDeviceExtension&)*vtExtensionPtr;
        }

        // be occurate with "VkDeviceCreateInfo", because after creation device, all "vt" extended structures will destoyed
        VkDevice vkDevice;
        if (vkCreateDevice(*physicalDevice, (const VkDeviceCreateInfo*)vtExplodeArtificals(vdvi), nullptr, &vkDevice) == VK_SUCCESS) { result = VK_SUCCESS; };

        // manually convert device
        convertDevice(vkDevice, physicalDevice, vtExtension, _vtDevice);
        return result;
    };


};
