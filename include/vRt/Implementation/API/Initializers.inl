#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vt;

    // TODO merge some extra functions to another headers


    uint32_t VtIdentifier = 0x1FFu;
    struct VkShortHead {
        //uint32_t sublevel : 24, identifier : 12;
        uint32_t sublevel : 12, identifier : 24; // I don't know ordering of bits
        uintptr_t pNext; // pull of C++20 raw pointers
    };

    struct VkFullHead {
        uint32_t sType;
        uintptr_t pNext; // pull of C++20 raw pointers
    };


    template <class VtS>
    auto vtSearchStructure(VtS& structure, VtStructureType sType) {
        VkFullHead* head = (VkFullHead*)&structure;
        VkFullHead* found = nullptr;
        for (int i = 0; i < 255; i++) {
            if (!head) break;
            if (head->sType == sType) {
                found = head; break;
            }
            head = (VkFullHead*)head->pNext;
        }
        return found;
    };

    template <class VtS>
    auto vtExplodeArtificals(VtS& structure) {
        VkShortHead* head = (VkShortHead*)&structure;
        VkShortHead* lastVkStructure = nullptr;
        VkShortHead* firstVkStructure = nullptr;
        for (int i = 0; i < 255; i++) {
            if (!head) break;
            if (head->identifier != VtIdentifier) {
                if (lastVkStructure) {
                    lastVkStructure->pNext = (uintptr_t)head;
                }
                else {
                    firstVkStructure = head;
                }
                lastVkStructure = head;
            }
            head = (VkShortHead*)head->pNext;
        }
        return firstVkStructure;
    };


    inline VtResult makePhysicalDevice(std::shared_ptr<Instance> instance, VkPhysicalDevice physical, std::shared_ptr<PhysicalDevice>& _vtPhysicalDevice){
        _vtPhysicalDevice = std::make_shared<PhysicalDevice>();
        _vtPhysicalDevice->_physicalDevice = physical; // assign a Vulkan physical device
        return VK_SUCCESS;
    };

    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_ONLY>
    inline VtResult createBuffer(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>>& _vtBuffer) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        auto& vtDeviceBuffer = (_vtBuffer = std::make_shared<RoledBuffer<U>>());
        vtDeviceBuffer->_device = device; // delegate device by weak_ptr

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = U;

        // make memory usages 
        auto usageFlag = cinfo.usageFlag;
        if constexpr (U != VMA_MEMORY_USAGE_GPU_ONLY) { allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; }
        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { usageFlag |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT; } else // from src only
        if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) { usageFlag |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT; } else // to dst only
        { usageFlag |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT; } // bidirectional

        auto binfo = VkBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, cinfo.bufferSize, usageFlag, VK_SHARING_MODE_EXCLUSIVE, 1, &cinfo.familyIndex };
        if (vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo) == VK_SUCCESS) { result = VK_SUCCESS; };
        vtDeviceBuffer->_size = cinfo.bufferSize;

        // if format is known, make bufferView
        if constexpr (U == VMA_MEMORY_USAGE_GPU_ONLY) { // spaghetti code, because had different qualifiers
            if (result == VK_SUCCESS && cinfo.format) {
                vtDeviceBuffer->_bufferView;
                VkBufferViewCreateInfo bvi;
                bvi.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
                bvi.buffer = vtDeviceBuffer->_buffer;
                bvi.format = cinfo.format;
                bvi.offset = 0;
                bvi.range = VK_WHOLE_SIZE;
                if (vkCreateBufferView(device->_device, &bvi, nullptr, &vtDeviceBuffer->_bufferView) == VK_SUCCESS) {
                    result = VK_SUCCESS;
                }
                else {
                    result = VK_INCOMPLETE;
                };
            }
        }

        return result;
    };


    // artifical function type
    template<VmaMemoryUsage U>
    using _createBuffer_T = VtResult(*)(std::shared_ptr<Device> device, const VtDeviceBufferCreateInfo& cinfo, std::shared_ptr<RoledBuffer<U>> &_vtBuffer);

    // aliased calls
    constexpr _createBuffer_T<VMA_MEMORY_USAGE_GPU_ONLY> createDeviceBuffer = &createBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr _createBuffer_T<VMA_MEMORY_USAGE_CPU_TO_GPU> createHostToDeviceBuffer = &createBuffer<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr _createBuffer_T<VMA_MEMORY_USAGE_GPU_TO_CPU> createDeviceToHostBuffer = &createBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;


    inline VtResult createDeviceImage(std::shared_ptr<Device> device, const VtDeviceImageCreateInfo& cinfo, std::shared_ptr<DeviceImage>& _vtImage) {
        // result will no fully handled
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        auto& texture = (_vtImage = std::make_shared<DeviceImage>());
        texture->_device = device; // delegate device by weak_ptr
        texture->_layout = (VkImageLayout)cinfo.layout;

        // init image dimensional type
        vk::ImageType imageType = vk::ImageType::e2D; bool isCubemap = false;
        switch (vk::ImageViewType(cinfo.imageViewType)) {
        case vk::ImageViewType::e1D:
            imageType = vk::ImageType::e1D;
            break;
        case vk::ImageViewType::e1DArray:
            imageType = vk::ImageType::e2D;
            break;
        case vk::ImageViewType::e2D:
            imageType = vk::ImageType::e2D;
            break;
        case vk::ImageViewType::e2DArray:
            imageType = vk::ImageType::e3D;
            break;
        case vk::ImageViewType::e3D:
            imageType = vk::ImageType::e3D;
            break;
        case vk::ImageViewType::eCube:
            imageType = vk::ImageType::e3D;
            isCubemap = true;
            break;
        case vk::ImageViewType::eCubeArray:
            imageType = vk::ImageType::e3D;
            isCubemap = true;
            break;
        };

        // image memory descriptor
        auto imageInfo = vk::ImageCreateInfo();
        imageInfo.initialLayout = vk::ImageLayout(texture->_initialLayout);
        imageInfo.imageType = imageType;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.arrayLayers = 1; // unsupported
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.extent = { cinfo.size.width, cinfo.size.height, cinfo.size.depth * (isCubemap ? 6 : 1) };
        imageInfo.format = vk::Format(cinfo.format);
        imageInfo.mipLevels = cinfo.mipLevels;
        imageInfo.pQueueFamilyIndices = &cinfo.familyIndex;
        imageInfo.queueFamilyIndexCount = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1; // at now not supported MSAA
        imageInfo.usage = vk::ImageUsageFlags(cinfo.usage);

        // create image with allocation
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (vmaCreateImage(device->_allocator, &(VkImageCreateInfo)imageInfo, &allocCreateInfo, (VkImage *)&texture->_image, &texture->_allocation, &texture->_allocationInfo) == VK_SUCCESS) { result = VK_SUCCESS; };

        // subresource range
        texture->_subresourceRange.levelCount = 1;
        texture->_subresourceRange.layerCount = 1;
        texture->_subresourceRange.baseMipLevel = 0;
        texture->_subresourceRange.baseArrayLayer = 0;
        texture->_subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // subresource layers
        texture->_subresourceLayers.layerCount = texture->_subresourceRange.layerCount;
        texture->_subresourceLayers.baseArrayLayer = texture->_subresourceRange.baseArrayLayer;
        texture->_subresourceLayers.aspectMask = texture->_subresourceRange.aspectMask;
        texture->_subresourceLayers.mipLevel = texture->_subresourceRange.baseMipLevel;

        // descriptor for usage 
        // (unhandled by vtResult)
        texture->_imageView = vk::Device(device->_device).createImageView(vk::ImageViewCreateInfo()
            .setSubresourceRange(texture->_subresourceRange)
            .setViewType(vk::ImageViewType(cinfo.imageViewType))
            .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
            .setImage(texture->_image)
            .setFormat(vk::Format(cinfo.format)));

        return result;
    };


    inline VtResult createRadixSort(std::shared_ptr<Device> _vtDevice, std::shared_ptr<RadixSort>& _vtRadix) {
        auto& vtRadix = (_vtRadix = std::make_shared<RadixSort>());
        vtRadix->_device = _vtDevice;

        VtDeviceBufferCreateInfo bfi;
        bfi.familyIndex = _vtDevice->_mainFamilyIndex;
        bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

        bfi.bufferSize = 1024 * 1024 * 32;
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_tmpValuesBuffer);

        bfi.bufferSize = 1024 * 1024 * 64;
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_tmpKeysBuffer);

        bfi.bufferSize = 1024;
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_stepsBuffer); // unused
        
        bfi.bufferSize = 1024 * 1024;
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_histogramBuffer);
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_prefixSumBuffer);

        std::vector<vk::PushConstantRange> constRanges = {
            vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
        };

        std::vector<vk::DescriptorSetLayout> dsLayouts = {
            vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["radixSort"]),
            vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["radixSortBind"])
        };

        vtRadix->_pipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
        vtRadix->_histogramPipeline = createCompute(*_vtDevice, _vtDevice->_shadersPath + "radix/histogram.comp.spv", vtRadix->_pipelineLayout, *_vtDevice);
        vtRadix->_workPrefixPipeline = createCompute(*_vtDevice, _vtDevice->_shadersPath + "radix/pfx-work.comp.spv", vtRadix->_pipelineLayout, *_vtDevice);
        vtRadix->_permutePipeline = createCompute(*_vtDevice, _vtDevice->_shadersPath + "radix/permute.comp.spv", vtRadix->_pipelineLayout, *_vtDevice);

        auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
        vtRadix->_descriptorSet = dsc[0];

        // write radix sort descriptor sets
        vk::WriteDescriptorSet _write_tmpl = vk::WriteDescriptorSet(vtRadix->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_stepsBuffer->_descriptorInfo())), //unused
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_tmpKeysBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_tmpValuesBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_histogramBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_prefixSumBuffer->_descriptorInfo())),
        };
    };



    inline VtResult createDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkDeviceCreateInfo& vdvi, std::shared_ptr<Device>& _vtDevice){
        auto& vtDevice = (_vtDevice = std::make_shared<Device>());
        vtDevice->_physicalDevice = physicalDevice; // reference for aliasing

        VtResult result = VK_ERROR_INITIALIZATION_FAILED;
        VtArtificalDeviceExtension vtExtension; // default structure values
        auto vtExtensionPtr = vtSearchStructure(vdvi, VT_STRUCTURE_TYPE_ARTIFICAL_DEVICE_EXTENSION);
        if (vtExtensionPtr) { // if found, getting some info
            vtExtension = (VtArtificalDeviceExtension&)*vtExtensionPtr;
        }

        // be occurate with "VkDeviceCreateInfo", because after creation device, all "vt" extended structures will destoyed
        if (vkCreateDevice(*(vtDevice->_physicalDevice.lock()), (const VkDeviceCreateInfo*)vtExplodeArtificals(vdvi), nullptr, &vtDevice->_device) == VK_SUCCESS) { result = VK_SUCCESS; };

        VmaAllocatorCreateInfo allocatorInfo;
        allocatorInfo.physicalDevice = *(vtDevice->_physicalDevice.lock());
        allocatorInfo.device = vtDevice->_device;
        allocatorInfo.preferredLargeHeapBlockSize = 16384; // 16kb
        allocatorInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;
        if (vmaCreateAllocator(&allocatorInfo, &vtDevice->_allocator) == VK_SUCCESS) { result = VK_SUCCESS; };

        // link device with vulkan.hpp
        auto& _device = vk::Device(vtDevice->_device);

        // create default pipeline cache
        vtDevice->_pipelineCache = VkPipelineCache(_device.createPipelineCache(vk::PipelineCacheCreateInfo()));
        
        // make descriptor pool
        std::vector<vk::DescriptorPoolSize> dps = {
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 8),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 32),
            vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 32),
            vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 256),
            vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 32),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 256),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 4),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 8),
        };
        vtDevice->_descriptorPool = VkDescriptorPool(_device.createDescriptorPool(vk::DescriptorPoolCreateInfo().setMaxSets(128).setPPoolSizes(dps.data()).setPoolSizeCount(dps.size())));
        vtDevice->_mainFamilyIndex = vtExtension.mainQueueFamily;

        // make traffic buffers 
        VtDeviceBufferCreateInfo dbfi;
        dbfi.bufferSize = tiled(vtExtension.sharedCacheSize, sizeof(uint32_t));
        dbfi.format = VkFormat(vk::Format::eR8Uint); // just uint8_t data
        dbfi.familyIndex = vtExtension.mainQueueFamily;
        createHostToDeviceBuffer(vtDevice, dbfi, vtDevice->_uploadBuffer);
        createDeviceToHostBuffer(vtDevice, dbfi, vtDevice->_downloadBuffer);


        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // rays
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // hit heads
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // closest hit indices
                vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // miss hit indices
                vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // hit payloads
                vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // ray traversal cache
                vk::DescriptorSetLayoutBinding(6, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // constant buffer
                vk::DescriptorSetLayoutBinding(7, vk::DescriptorType::eStorageBufferDynamic, 1, vk::ShaderStageFlagBits::eCompute), // counters 
                vk::DescriptorSetLayoutBinding(8, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // 9-line 
            };
            vtDevice->_descriptorLayoutMap["rayTracing"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // morton codes
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // morton indices
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // 
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // leafs
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // aabb on work
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vote flags
                vk::DescriptorSetLayoutBinding(6 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process indices
                vk::DescriptorSetLayoutBinding(7 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // leaf node indices
                vk::DescriptorSetLayoutBinding(8 , vk::DescriptorType::eStorageBufferDynamic, 1, vk::ShaderStageFlagBits::eCompute), // in-process counters
                vk::DescriptorSetLayoutBinding(9 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // in-process scene box
                vk::DescriptorSetLayoutBinding(10, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh uniform block
                vk::DescriptorSetLayoutBinding(11, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh meta 
                vk::DescriptorSetLayoutBinding(12, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // box boxes 
                vk::DescriptorSetLayoutBinding(13, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // bvh meta (bufferView)
            };
            vtDevice->_descriptorLayoutMap["hlbvh2"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        {
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eStorageBufferDynamic, 1, vk::ShaderStageFlagBits::eCompute), // vertex assembly counters
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // material buffer (unused)
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // order buffer (unused)
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // writable vertices
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute), // writable attributes
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // readonly vertices
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
                vk::DescriptorSetLayoutBinding(0 , vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vertex raw data
                vk::DescriptorSetLayoutBinding(1 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // virtual regions
                vk::DescriptorSetLayoutBinding(2 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // buffer views
                vk::DescriptorSetLayoutBinding(3 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // accessors
                vk::DescriptorSetLayoutBinding(4 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // attribute bindings 
                vk::DescriptorSetLayoutBinding(5 , vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // vertex input uniform
            };
            vtDevice->_descriptorLayoutMap["vertexInputSet"] = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        }

        return result;
    };





    // transition texture layout
    inline VtResult imageBarrier(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> image) {
        VtResult result = VK_SUCCESS; // planned to complete

        vk::ImageMemoryBarrier imageMemoryBarriers = {};
        imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.oldLayout = (vk::ImageLayout)image->_initialLayout;
        imageMemoryBarriers.newLayout = (vk::ImageLayout)image->_layout;
        imageMemoryBarriers.image = image->_image;
        imageMemoryBarriers.subresourceRange = image->_subresourceRange;

        // Put barrier on top
        vk::PipelineStageFlags srcStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
        vk::PipelineStageFlags dstStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
        vk::DependencyFlags dependencyFlags{};
        vk::AccessFlags srcMask{};
        vk::AccessFlags dstMask{};

        typedef vk::ImageLayout il;
        typedef vk::AccessFlagBits afb;

        // Is it me, or are these the same?
        switch ((vk::ImageLayout)image->_initialLayout) {
            case il::eUndefined: break;
            case il::eGeneral: srcMask = afb::eTransferWrite; break;
            case il::eColorAttachmentOptimal: srcMask = afb::eColorAttachmentWrite; break;
            case il::eDepthStencilAttachmentOptimal: srcMask = afb::eDepthStencilAttachmentWrite; break;
            case il::eDepthStencilReadOnlyOptimal: srcMask = afb::eDepthStencilAttachmentRead; break;
            case il::eShaderReadOnlyOptimal: srcMask = afb::eShaderRead; break;
            case il::eTransferSrcOptimal: srcMask = afb::eTransferRead; break;
            case il::eTransferDstOptimal: srcMask = afb::eTransferWrite; break;
            case il::ePreinitialized: srcMask = afb::eTransferWrite | afb::eHostWrite; break;
            case il::ePresentSrcKHR: srcMask = afb::eMemoryRead; break;
        }

        switch ((vk::ImageLayout)image->_layout) {
            case il::eUndefined: break;
            case il::eGeneral: dstMask = afb::eTransferWrite; break;
            case il::eColorAttachmentOptimal: dstMask = afb::eColorAttachmentWrite; break;
            case il::eDepthStencilAttachmentOptimal: dstMask = afb::eDepthStencilAttachmentWrite; break;
            case il::eDepthStencilReadOnlyOptimal: dstMask = afb::eDepthStencilAttachmentRead; break;
            case il::eShaderReadOnlyOptimal: dstMask = afb::eShaderRead; break;
            case il::eTransferSrcOptimal: dstMask = afb::eTransferRead; break;
            case il::eTransferDstOptimal: dstMask = afb::eTransferWrite; break;
            case il::ePreinitialized: dstMask = afb::eTransferWrite; break;
            case il::ePresentSrcKHR: dstMask = afb::eMemoryRead; break;
        }

        // assign access masks
        imageMemoryBarriers.srcAccessMask = srcMask;
        imageMemoryBarriers.dstAccessMask = dstMask;

        // barrier
        vk::CommandBuffer(cmd).pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlagBits::eByRegion, {}, {}, std::array<vk::ImageMemoryBarrier, 1>{imageMemoryBarriers});
        image->_initialLayout = (VkImageLayout)imageMemoryBarriers.newLayout;

        return result;
    };




    // copy buffer command with inner "DeviceBuffer"
    inline void cmdCopyBuffer(VkCommandBuffer cmd, std::shared_ptr<DeviceBuffer> srcBuffer, std::shared_ptr<DeviceBuffer> dstBuffer, const std::vector<vk::BufferCopy>& regions) {
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions);
    };

    // copy to host buffer
    // you can't use it for form long command buffer to host
    inline void cmdCopyBufferToHost(VkCommandBuffer cmd, std::shared_ptr<DeviceBuffer> srcBuffer, std::shared_ptr<DeviceToHostBuffer> dstBuffer, const std::vector<vk::BufferCopy>& regions) {
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions, toHostCommandBarrier);
    };

    // copy from host buffer
    // you can't use it for form long command buffer from host
    inline void cmdCopyBufferFromHost(VkCommandBuffer cmd, std::shared_ptr<DeviceToHostBuffer> srcBuffer, std::shared_ptr<DeviceBuffer> dstBuffer, const std::vector<vk::BufferCopy>& regions) {
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions, fromHostCommandBarrier);
    };


    // copy buffer to image (from gpu or host)
    // you can't use it for form long command buffer from host
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_CPU_TO_GPU>
    inline void cmdCopyBufferToImage(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> srcBuffer, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::BufferImageCopy>& regions) {
        vk::CommandBuffer(cmd).copyBufferToImage((vk::Buffer&)(srcBuffer->_buffer), (vk::Image&)(dstImage->_image), vk::ImageLayout(dstImage->_layout), regions);

        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { fromHostCommandBarrier(cmd); } else 
        if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) {   toHostCommandBarrier(cmd); } else
        { commandBarrier(cmd); }
    };

    // copy image to buffer (to gpu or host)
    // you can't use it for form long command buffer to host
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_TO_CPU>
    inline void cmdCopyImageToBuffer(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<RoledBuffer<U>> dstBuffer, const std::vector<vk::BufferImageCopy>& regions) {
        vk::CommandBuffer(cmd).copyImageToBuffer((vk::Image&)(srcImage->_image), vk::ImageLayout(srcImage->_layout), (vk::Buffer&)(dstBuffer->_buffer), regions);

        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { fromHostCommandBarrier(cmd); } else
        if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) {   toHostCommandBarrier(cmd); } else
        { commandBarrier(cmd); }
    };


    template<VmaMemoryUsage U>
    using cmdCopyImageToBuffer_T = void(*)(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<RoledBuffer<U>> dstBuffer, const std::vector<vk::BufferImageCopy>& regions);

    template<VmaMemoryUsage U>
    using cmdCopyBufferToImage_T = void(*)(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> srcBuffer, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::BufferImageCopy>& regions);

    // aliased low level calls
    constexpr cmdCopyBufferToImage_T<VMA_MEMORY_USAGE_GPU_ONLY> copyDeviceBufferToImage = &cmdCopyBufferToImage<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr cmdCopyImageToBuffer_T<VMA_MEMORY_USAGE_GPU_ONLY> copyImageToDeviceBuffer = &cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr cmdCopyBufferToImage_T<VMA_MEMORY_USAGE_CPU_TO_GPU> copyHostBufferToImage = &cmdCopyBufferToImage<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr cmdCopyImageToBuffer_T<VMA_MEMORY_USAGE_GPU_TO_CPU> copyImageToHostBuffer = &cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;



    // direct state commands for loading data

    // set buffer data function (defaultly from HostToDevice)
    template <class T, VmaMemoryUsage U = VMA_MEMORY_USAGE_CPU_TO_GPU>
    inline void setBufferSubData(const std::vector<T> &hostdata, std::shared_ptr<RoledBuffer<U>> buffer, intptr_t offset = 0) {
        const size_t bufferSize = hostdata.size() * sizeof(T);
        if (bufferSize > 0) memcpy((uint8_t *)buffer->_hostMapped() + offset, hostdata.data(), bufferSize);
    }

    // get buffer data function (defaultly from DeviceToHost)
    template <class T, VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_TO_CPU>
    inline void getBufferSubData(std::shared_ptr<RoledBuffer<U>> buffer, std::vector<T> &hostdata, intptr_t offset = 0) {
        if (hostdata.size() > 0) memcpy(hostdata.data(), (const uint8_t *)buffer->_hostMapped() + offset, hostdata.size() * sizeof(T));
    }

    // get buffer data function (defaultly from DeviceToHost)
    template <class T, VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_TO_CPU>
    inline auto getBufferSubData(std::shared_ptr<RoledBuffer<U>> buffer, size_t count = 1, intptr_t offset = 0) {
        std::vector<T> hostdata(count);
        if (hostdata.size() > 0) memcpy(hostdata.data(), (const uint8_t *)buffer->_hostMapped() + offset, count * sizeof(T));
        return hostdata; // in return will copying, C++ does not made mechanism for zero-copy of anything
    }

};
