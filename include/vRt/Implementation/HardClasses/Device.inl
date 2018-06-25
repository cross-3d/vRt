#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;


    inline auto getVendorName(const uint32_t& vendorID) {
        auto shaderDir = VT_VENDOR_UNIVERSAL;
        switch (vendorID) {
            case 4318:
                shaderDir = VT_VENDOR_NVIDIA;
                break;
            case 4098:
                shaderDir = VT_VENDOR_AMD;
                break;
            case 8086: // x86 ID, WHAT?
                shaderDir = VT_VENDOR_INTEL;
                break;
        }
        return shaderDir;
    }


    inline VtResult convertDevice(VkDevice device, std::shared_ptr<PhysicalDevice> physicalDevice, const VtArtificalDeviceExtension& vtExtension, std::shared_ptr<Device>& _vtDevice) {
        auto& vtDevice = (_vtDevice = std::make_shared<Device>());
        vtDevice->_physicalDevice = physicalDevice; // reference for aliasing
        vtDevice->_device = device;

        vk::PhysicalDevice gpu = *vtDevice->_physicalDevice;
        vtDevice->_vendorName = getVendorName(gpu.getProperties().vendorID);

        VtResult result = VK_SUCCESS;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = *(vtDevice->_physicalDevice);
        allocatorInfo.device = vtDevice->_device;
        allocatorInfo.preferredLargeHeapBlockSize = 16384; // 16kb
        allocatorInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocatorInfo.pAllocationCallbacks = nullptr;
        allocatorInfo.pVulkanFunctions = nullptr;
        allocatorInfo.pHeapSizeLimit = nullptr;

        if (vtExtension.allocator) {
            vtDevice->_allocator = vtExtension.allocator; result = VK_SUCCESS;
        } else {
            if (vmaCreateAllocator(&allocatorInfo, &vtDevice->_allocator) == VK_SUCCESS) { result = VK_SUCCESS; };
        }

        // link device with vulkan.hpp
        auto _device = vk::Device(vtDevice->_device);

        // create default pipeline cache
        vtDevice->_pipelineCache = VkPipelineCache(_device.createPipelineCache(vk::PipelineCacheCreateInfo()));

        // make descriptor pool
        size_t mult = 8;
        std::vector<vk::DescriptorPoolSize> dps = {
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 8 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 32 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 32 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 256 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 32 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 256 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 4 * mult),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 8 * mult),
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
                vk::DescriptorSetLayoutBinding(9, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // traverse cache (just have no idea to bind)
                vk::DescriptorSetLayoutBinding(10, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray<->hit binding payload 
                vk::DescriptorSetLayoutBinding(11, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute),
            };
            vtDevice->_descriptorLayoutMap["rayTracing"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh uniform block
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh meta 
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // box boxes 
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh meta (bufferView)
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
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // radice step properties
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // keys cache
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values cache
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // histogram of radices (every work group)
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // prefix-sum of radices (every work group)
            };
            vtDevice->_descriptorLayoutMap["radixSort"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eSampledImage, 64, vk::ShaderStageFlagBits::eCompute), // textures
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eSampler, 16, vk::ShaderStageFlagBits::eCompute), // samplers
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material buffer
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // virtual texture and sampler combinations
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material set uniform 
            };
            vtDevice->_descriptorLayoutMap["materialSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eUniformTexelBuffer, 8, vk::ShaderStageFlagBits::eCompute), // vertex raw data
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // virtual regions
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // buffer views
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // accessors
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // attribute bindings 
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vertex input uniform
            };
            vtDevice->_descriptorLayoutMap["vertexInputSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }


        const auto& vendorName = _vtDevice->_vendorName;

        //createPipelineLayout
        VtVertexAssemblyPipelineCreateInfo simfo;
        simfo.vertexAssemblyModule = loadAndCreateShaderModuleStage(*vtDevice, vt::natives::vertexAssembly[vendorName]);
        simfo.maxPrimitives = vtExtension.maxPrimitives;
        createPipelineLayout(vtDevice, vk::PipelineLayoutCreateInfo(), simfo.pipelineLayout, VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT);

        // create radix sort tool
        createRadixSort(vtDevice, vtExtension, vtDevice->_radixSort);
        createVertexAssemblyPipeline(vtDevice, simfo, vtDevice->_vertexAssembler);
        createAcceleratorHLBVH2(vtDevice, vtExtension, vtDevice->_acceleratorBuilder);

        // create dull barrier pipeline
        auto rng = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2));
        auto ppl = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, 0, nullptr, 0, nullptr));
        vtDevice->_dullBarrier = createComputeMemory(VkDevice(*_vtDevice), natives::dullBarrier[vtDevice->_vendorName], ppl, VkPipelineCache(*_vtDevice));

        return result;
    };


    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo vdvi, std::shared_ptr<Device>& _vtDevice) {
        //auto& vtDevice = (_vtDevice = std::make_shared<Device>());
        //vtDevice->_physicalDevice = physicalDevice; // reference for aliasing

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
