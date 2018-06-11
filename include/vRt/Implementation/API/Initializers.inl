#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vt;


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
        { 
            usageFlag |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
            if (cinfo.format) { // if has format, add texel storage usage
                usageFlag |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
            }
        } // bidirectional

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

        // additional usage
        auto usage = vk::ImageUsageFlags(cinfo.usage) | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

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
        imageInfo.usage = usage;

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
        vtRadix->_histogramPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "radix/histogram.comp.spv", vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_workPrefixPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "radix/pfx-work.comp.spv", vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_permutePipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "radix/permute.comp.spv", vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));

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
        vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
    };



    inline VtResult createVertexAssembly(std::shared_ptr<Device> _vtDevice, const VtAcceleratorCreateInfo &info, std::shared_ptr<VertexAssembly>& _vtVertexAssembly) {
        VtResult result = VK_SUCCESS;
        auto& vtVertexAssembly = (_vtVertexAssembly = std::make_shared<VertexAssembly>());
        vtVertexAssembly->_device = _vtDevice;

        constexpr auto maxPrimitives = 1024u * 1024u; // planned import from descriptor

        // build vertex input assembly program
        {
            constexpr auto ATTRIB_EXTENT = 4u; // no way to set more than it now

            VtDeviceBufferCreateInfo bfi;
            bfi.familyIndex = _vtDevice->_mainFamilyIndex;
            bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

            // vertex data buffers
            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_UNDEFINED;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_orderBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_UNDEFINED;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_materialBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(float) * 4;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_verticeBuffer);

            // create vertex attribute buffer
            VtDeviceImageCreateInfo tfi;
            tfi.familyIndex = _vtDevice->_mainFamilyIndex;
            tfi.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            tfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            tfi.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
            tfi.layout = VK_IMAGE_LAYOUT_GENERAL;
            tfi.mipLevels = 1;
            tfi.size = { 6144u, tiled(maxPrimitives * 3u * ATTRIB_EXTENT, 6144u) };
            createDeviceImage(_vtDevice, tfi, vtVertexAssembly->_attributeTexelBuffer);
        };

        {
            std::vector<vk::PushConstantRange> constRanges = {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(4))
            };
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
            };
            auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtVertexAssembly->_vertexAssemblyDescriptorSet = dsc[0];

            auto _write_tmpl = vk::WriteDescriptorSet(vtVertexAssembly->_vertexAssemblyDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                // TODO write
                //vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonCodesBuffer->_descriptorInfo())), //unused
                //vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonIndicesBuffer->_descriptorInfo()))
            };
            vk::Device(*_vtDevice).updateDescriptorSets(_write_tmpl, {});


            vtVertexAssembly->_vertexAssemblyPipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
            vtVertexAssembly->_vertexAssemblyPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "utils/vinput.comp.spv", vtVertexAssembly->_vertexAssemblyPipelineLayout, VkPipelineCache(*_vtDevice));
        };

        return result;
    };


    inline VtResult createAccelerator(std::shared_ptr<Device> _vtDevice, const VtAcceleratorCreateInfo &info, std::shared_ptr<Accelerator>& _vtAccelerator) {
        VtResult result = VK_SUCCESS;
        auto& vtAccelerator = (_vtAccelerator = std::make_shared<Accelerator>());
        vtAccelerator->_device = _vtDevice;

        // planned import from descriptor
        constexpr auto maxPrimitives = 1024u * 1024u;

        // build BVH builder program
        {
            {
                std::vector<vk::PushConstantRange> constRanges = {
                    vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
                };
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"])
                };
                vtAccelerator->_buildPipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
                auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtAccelerator->_buildDescriptorSet = dsc[0];
            };

            {
                std::vector<vk::PushConstantRange> constRanges = {
                    //vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
                };
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                };
                vtAccelerator->_traversePipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
                vtAccelerator->_traverseDescriptorSet = vtAccelerator->_buildDescriptorSet;
            };

            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint64_t);
                bfi.format = VK_FORMAT_R32G32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_mortonCodesBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_SINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_mortonIndicesBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 4 * 2;
                bfi.format = VK_FORMAT_R32G32B32A32_SINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_bvhMetaBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16 * 2;
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_bvhBoxBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16 * 2;
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_boundaryResultBuffer);

                bfi.bufferSize = sizeof(uint32_t) * 8;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_bvhBlockUniform);

                bfi.bufferSize = maxPrimitives * 2 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_currentNodeIndices);

                bfi.bufferSize = maxPrimitives * 2 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_fitStatusBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_leafNodeIndices);

                bfi.bufferSize = 16 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_countersBuffer);

                bfi.bufferSize = 128 * 16 * sizeof(float);
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_generalBoundaryResultBuffer);
            };

            {
                auto _write_tmpl = vk::WriteDescriptorSet(vtAccelerator->_buildDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(11).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtAccelerator->_bvhMetaBuffer->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(13).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&vk::BufferView(vtAccelerator->_bvhMetaBuffer->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonCodesBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonIndicesBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_leafBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_bvhBoxBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_fitStatusBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_currentNodeIndices->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(7).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_leafNodeIndices->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(8).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_countersBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(9).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_generalBoundaryResultBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(10).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_bvhBlockUniform->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(12).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_boundaryResultBuffer->_descriptorInfo())),
                };
                vk::Device(*_vtDevice).updateDescriptorSets(_write_tmpl, {});
            };

            // create pipelines (planned to unify between accelerator instances)
            {
                vtAccelerator->_boundingPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlbvh2/bound-calc.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_buildPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlbvh2/bvh-build.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_fitPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlbvh2/bvh-fit.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_leafPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlbvh2/leaf-gen.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_intersectionPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlbvh2/traverse-bvh.comp.spv", vtAccelerator->_traversePipelineLayout, VkPipelineCache(*_vtDevice));
            };
        };


        // write radix sort descriptor sets
        {
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["radixSortBind"]),
            };
            auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_sortDescriptorSet = dsc[0];

            auto _write_tmpl = vk::WriteDescriptorSet(vtAccelerator->_sortDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonCodesBuffer->_descriptorInfo())), //unused
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonIndicesBuffer->_descriptorInfo()))
            };
            vk::Device(*_vtDevice).updateDescriptorSets(_write_tmpl, {});
        };


        // create or link vertex assembly
        createVertexAssembly(_vtDevice, info, vtAccelerator->_vertexAssembly);

        return result;
    };


    // TODO - add support for auto-creation of buffers in "VtVertexInputCreateInfo" from pointers and counts
    // also, planned to add support of offsets in buffers 
    inline VtResult createVertexInputSet(std::shared_ptr<Device> _vtDevice, VtVertexInputCreateInfo& info, std::shared_ptr<VertexInputSet>& _vtVertexInput) {
        VtResult result = VK_SUCCESS;
        auto& vtVertexInput = (_vtVertexInput = std::make_shared<VertexInputSet>());
        vtVertexInput->_device = _vtDevice;
        
        std::vector<vk::PushConstantRange> constRanges = {
            vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(4))
        };
        std::vector<vk::DescriptorSetLayout> dsLayouts = {
            vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
        };
        auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
        vtVertexInput->_descriptorSet = dsc[0];

        // 
        VtDeviceBufferCreateInfo bfi;
        bfi.familyIndex = _vtDevice->_mainFamilyIndex;
        bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);
        bfi.bufferSize = sizeof(uint32_t) * 8;
        bfi.format = VK_FORMAT_UNDEFINED;

        // planned add external buffer support
        createDeviceBuffer(_vtDevice, bfi, vtVertexInput->_uniformBlockBuffer);
        _vtDevice->_deviceBuffersPtrs.push_back(vtVertexInput->_uniformBlockBuffer); // pin buffer with device

        // set primitive count (will loaded to "_uniformBlockBuffer" by cmdUpdateBuffer)
        vtVertexInput->_uniformBlock.primitiveCount = info.primitiveCount;
        vtVertexInput->_uniformBlock.verticeAccessor = info.verticeAccessor;
        vtVertexInput->_uniformBlock.indiceAccessor = info.indiceAccessor;
        vtVertexInput->_uniformBlock.materialID = info.materialID;

        // write descriptors
        auto _write_tmpl = vk::WriteDescriptorSet(vtVertexInput->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&vk::BufferView(vtVertexInput->_dataSourceBuffer->_bufferView)),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferRegionBindings->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferViews->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferAccessors->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferAttributeBindings->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_uniformBlockBuffer->_descriptorInfo())),
        };
        vk::Device(*_vtDevice).updateDescriptorSets(_write_tmpl, {});

        return result;
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
        
        // create radix sort tool
        createRadixSort(vtDevice, vtDevice->_radixSort);

        return result;
    };

};
