#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vrt;

    VtResult createAcceleratorHLBVH2(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<AcceleratorHLBVH2>& vtAccelerator) {
        VtResult result = VK_SUCCESS;
        //auto vtAccelerator = (_vtAccelerator = std::make_shared<AcceleratorHLBVH2>());
        vtAccelerator = std::make_shared<AcceleratorHLBVH2>();
        vtAccelerator->_device = _vtDevice;
        auto vkDevice = _vtDevice->_device;
        auto vkPipelineCache = _vtDevice->_pipelineCache;
        const auto vendorName = _vtDevice->_vendorName;

        // build BVH builder program
        {
            {
                std::vector<vk::PushConstantRange> constRanges = {
                    vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
                };
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2work"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"])
                };

                // create pipeline layout
                vtAccelerator->_buildPipelineLayout = vk::Device(vkDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
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

                // create pipeline layout
                vtAccelerator->_traversePipelineLayout = vk::Device(vkDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
            };


            // create pipelines (planned to unify between accelerator instances)
            {
                vtAccelerator->_shorthandPipeline = createComputeHC(vkDevice, hlbvh2::shorthand.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_boundingPipeline = createComputeHC(vkDevice, hlbvh2::triangle::outerBox.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_buildPipeline = createComputeHC(vkDevice, hlbvh2::builder.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_buildPipelineFirst = createComputeHC(vkDevice, hlbvh2::builderFirst.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_fitPipeline = createComputeHC(vkDevice, hlbvh2::fitBox.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_leafPipeline = createComputeHC(vkDevice, hlbvh2::triangle::genLeafs.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_leafLinkPipeline = createComputeHC(vkDevice, hlbvh2::linkLeafs.at(vendorName), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_intersectionPipeline = createComputeHC(vkDevice, hlbvh2::traverse.at(vendorName), vtAccelerator->_traversePipelineLayout, vkPipelineCache);
                vtAccelerator->_interpolatorPipeline = createComputeHC(vkDevice, hlbvh2::interpolator.at(vendorName), vtAccelerator->_traversePipelineLayout, vkPipelineCache);
            };
        };

        return result;
    };

    VtResult createAcceleratorSet(std::shared_ptr<Device> _vtDevice, const VtAcceleratorSetCreateInfo &info, std::shared_ptr<AcceleratorSet>& _vtAccelerator) {
        VtResult result = VK_SUCCESS;
        auto vtAccelerator = (_vtAccelerator = std::make_shared<AcceleratorSet>());
        auto vkDevice = _vtDevice->_device;
        vtAccelerator->_device = _vtDevice;
        vtAccelerator->_entryID = (info.entryID>>1u)<<1u; // unpreferred to make entry ID non power of 2

        // planned import from descriptor
        const auto maxPrimitives = info.maxPrimitives;
        vtAccelerator->_primitiveCount = info.primitiveCount;
        vtAccelerator->_primitiveOffset = info.primitiveOffset;


        std::shared_ptr<BufferManager> bManager; createBufferManager(_vtDevice, bManager);

        VtDeviceBufferCreateInfo bfic;
        bfic.familyIndex = _vtDevice->_mainFamilyIndex;
        bfic.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

        VtBufferRegionCreateInfo bfi;

        if (!info.secondary)
        { // solve building BVH conflicts by creation in accelerator set
            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16ull;
            bfi.format = VK_FORMAT_UNDEFINED;
            createBufferRegion(bManager, bfi, vtAccelerator->_leafBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint64_t);
            bfi.format = VK_FORMAT_R32G32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_mortonCodesBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_mortonIndicesBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 64ull;
            bfi.format = VK_FORMAT_R32G32B32A32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_onWorkBoxes);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 64ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_currentNodeIndices);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 2ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_fitStatusBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_leafNodeIndices);

            bfi.bufferSize = sizeof(uint32_t) * 64ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_countersBuffer);

            bfi.bufferSize = sizeof(float) * 65536ull;
            bfi.format = VK_FORMAT_R32G32B32A32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_generalBoundaryResultBuffer);
        };

        // UNUSED
        if (!info.bvhMetaBuffer) {
            bfi.bufferSize = sizeof(uint32_t) * 32ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhMetaBuffer);
        }

        if (!info.bvhBoxBuffer) {
            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 128ull;
            bfi.format = VK_FORMAT_R32G32B32A32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhBoxBuffer);
        }

        {
            bfi.bufferSize = sizeof(uint32_t) * 2048ull;
            bfi.format = VK_FORMAT_UNDEFINED;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhBlockUniform);
        }

        { // build final shared buffer for this class
            createSharedBuffer(bManager, vtAccelerator->_sharedBuffer, bfic);
        }


        if (!info.secondary)
        {
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2work"])
            };

            // create pipeline layout
            auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_buildDescriptorSet = dsc[0];

            auto writeTmpl = vk::WriteDescriptorSet(vtAccelerator->_buildDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(8).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_countersBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_mortonCodesBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_mortonIndicesBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_leafBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_onWorkBoxes->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(5).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_fitStatusBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_currentNodeIndices->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_leafNodeIndices->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(9).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_generalBoundaryResultBuffer->_descriptorInfo()),
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };

        // write radix sort descriptor sets
        {
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["radixSortBind"]),
            };
            auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_sortDescriptorSet = dsc[0];

            auto writeTmpl = vk::WriteDescriptorSet(vtAccelerator->_sortDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_mortonCodesBuffer->_descriptorInfo()), //unused
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_mortonIndicesBuffer->_descriptorInfo())
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };

        { // build BVH builder program
            std::vector<vk::PushConstantRange> constRanges = {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
            };
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"])
            };

            // create descriptor set
            auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_descriptorSet = dsc[0];
        };

        {
            VkBufferView bvhMetaView = {};
            if (info.bvhMetaBuffer) {
                VkBufferViewCreateInfo bvi = {};
                bvi.pNext = nullptr;
                bvi.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
                bvi.flags = {};
                bvi.buffer = info.bvhMetaBuffer;
                bvi.format = VK_FORMAT_R32G32B32A32_UINT;
                bvi.offset = 4ull * sizeof(int32_t) * info.bvhMetaOffset;
                bvi.range = VK_WHOLE_SIZE;
                if (vkCreateBufferView(_vtDevice->_device, &bvi, nullptr, &bvhMetaView) == VK_SUCCESS) {
                    result = VK_SUCCESS;
                } else {
                    result = VK_INCOMPLETE;
                };
            };

            auto metaView = bvhMetaView ? vk::BufferView(bvhMetaView) : vk::BufferView(vtAccelerator->_bvhMetaBuffer->_bufferView());
            auto metaBufferDesc = info.bvhMetaBuffer ? vk::DescriptorBufferInfo(info.bvhMetaBuffer, 0, VK_WHOLE_SIZE) : vk::DescriptorBufferInfo(vtAccelerator->_bvhMetaBuffer->_descriptorInfo());
            metaBufferDesc.offset += 4ull * sizeof(int32_t) * info.bvhMetaOffset;

            auto boxBuffer = info.bvhBoxBuffer ? vk::DescriptorBufferInfo(info.bvhBoxBuffer, 16ull * sizeof(int32_t) * info.bvhBoxOffset, VK_WHOLE_SIZE) : vk::DescriptorBufferInfo(vtAccelerator->_bvhBoxBuffer->_descriptorInfo());
            auto writeTmpl = vk::WriteDescriptorSet(vtAccelerator->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                //vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&metaView),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&metaView),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo(&metaBufferDesc),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo(&boxBuffer),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)(&vtAccelerator->_bvhBlockUniform->_descriptorInfo())),
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };

        return result;
    };
};
