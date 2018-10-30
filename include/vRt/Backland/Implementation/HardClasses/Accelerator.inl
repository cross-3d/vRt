#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;

    // TODO: enable AABB shaders for real support of multi-leveling (i.e. top level)
    VtResult createAcceleratorHLBVH2(std::shared_ptr<Device> _vtDevice, VtArtificalDeviceExtension info, std::shared_ptr<AcceleratorHLBVH2>& vtAccelerator) {
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
                vtAccelerator->_boxCalcPipeline.push_back(createComputeHC(vkDevice, getCorrectPath(hlbvh2::instance::boxCalc, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache));
                vtAccelerator->_boxCalcPipeline.push_back(createComputeHC(vkDevice, getCorrectPath(hlbvh2::triangle::boxCalc, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache));

                vtAccelerator->_leafPipeline.push_back(createComputeHC(vkDevice, getCorrectPath(hlbvh2::instance::genLeaf, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache));
                vtAccelerator->_leafPipeline.push_back(createComputeHC(vkDevice, getCorrectPath(hlbvh2::triangle::genLeaf, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache));

                vtAccelerator->_shorthandPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::shorthand, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_boundingPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::outerBox, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_buildPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::builder, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_buildPipelineFirst = createComputeHC(vkDevice, getCorrectPath(hlbvh2::builderFirst, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_fitPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::fitBox, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache);

                vtAccelerator->_leafLinkPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::linkLeafs, vendorName, _vtDevice->_shadersPath), vtAccelerator->_buildPipelineLayout, vkPipelineCache);
                vtAccelerator->_intersectionPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::traverse, vendorName, _vtDevice->_shadersPath), vtAccelerator->_traversePipelineLayout, vkPipelineCache);
                //vtAccelerator->_interpolatorPipeline = createComputeHC(vkDevice, getCorrectPath(hlbvh2::interpolator, vendorName, _vtDevice->_shadersPath), vtAccelerator->_traversePipelineLayout, vkPipelineCache);
            };
        };


        

        VtBufferRegionCreateInfo bfi = {};
        const auto maxPrimitives = info.maxPrimitives;

        std::shared_ptr<BufferManager> bManager = {}; createBufferManager(_vtDevice, bManager);
        { // solve building BVH conflicts by creation in accelerator set
            bfi.bufferSize = std::max(VkDeviceSize(sizeof(uint32_t) * 64ull), VkDeviceSize(sizeof(VtBuildConst)));
            bfi.format = VK_FORMAT_UNDEFINED;
            createBufferRegion(bManager, bfi, vtAccelerator->_constBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16ull;
            bfi.format = VK_FORMAT_UNDEFINED;
            createBufferRegion(bManager, bfi, vtAccelerator->_leafBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_mortonIndicesBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_mortonCodesBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createBufferRegion(bManager, bfi, vtAccelerator->_onWorkBoxes);

            bfi.bufferSize = (maxPrimitives + 4ull) * sizeof(uint32_t) * 16ull * 4ull; // denends by work group counts 
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_currentNodeIndices);

            bfi.bufferSize = (maxPrimitives + 4ull) * sizeof(uint32_t) * 2ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_fitStatusBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_leafNodeIndices);

            bfi.bufferSize = sizeof(uint32_t) * 64ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createBufferRegion(bManager, bfi, vtAccelerator->_countersBuffer);

            bfi.bufferSize = sizeof(float) * 16ull * 256ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createBufferRegion(bManager, bfi, vtAccelerator->_generalBoundaryResultBuffer);
        };

        { // build final shared buffer for this class
            VtDeviceBufferCreateInfo bfic = {};
            bfic.familyIndex = _vtDevice->_mainFamilyIndex;
            bfic.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);
            createSharedBuffer(bManager, bfic, vtAccelerator->_sharedBuffer);
        };

        {
            std::vector<vk::DescriptorSetLayout> dsLayouts = { vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2work"]) };

            // create pipeline layout
            auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_buildDescriptorSet = dsc[0];

            auto writeTmpl = vk::WriteDescriptorSet(vtAccelerator->_buildDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_mortonCodesBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_mortonIndicesBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_constBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_leafBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_onWorkBoxes->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(5).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_fitStatusBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_currentNodeIndices->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_leafNodeIndices->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(8).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_countersBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(9).setPBufferInfo((vk::DescriptorBufferInfo*)&vtAccelerator->_generalBoundaryResultBuffer->_descriptorInfo()),
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };
        
        { // write radix sort descriptor sets
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

        return result;
    };



    AcceleratorSet::~AcceleratorSet() {
        if (_descriptorSet) vk::Device(VkDevice(*_device)).freeDescriptorSets(_device->_descriptorPool, { vk::DescriptorSet(_descriptorSet) });
        _descriptorSet = {};
    };


    // planned advanced accelerator construction too
    VtResult createAcceleratorSet(std::shared_ptr<Device> _vtDevice, VtAcceleratorSetCreateInfo info, std::shared_ptr<AcceleratorSet>& _vtAccelerator) {
        VtResult result = VK_SUCCESS;
        auto vtAccelerator = (_vtAccelerator = std::make_shared<AcceleratorSet>());
        auto vkDevice = _vtDevice->_device;
        vtAccelerator->_device = _vtDevice;
        vtAccelerator->_coverMatrice = info.coverMat;
        vtAccelerator->_elementsOffset = info.vertexPointingOffset;
        //vtAccelerator->_elementsCount = info.maxPrimitives;
        vtAccelerator->_capacity = info.maxPrimitives;
        vtAccelerator->_entryID = info.traversingEntryID;
        vtAccelerator->_level = info.structureLevel;

        // planned import from descriptor
        std::shared_ptr<BufferManager> bManager = {};
        createBufferManager(_vtDevice, bManager);

        // 
        VtBufferRegionCreateInfo bfi = {};
        if (!info.bvhDataBuffer) { 
            bfi.bufferSize = (info.maxPrimitives * sizeof(VtBvhNodeStruct)) << 1ull;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhBoxBuffer); 
        } else 
        { bfi.offset = info.bvhDataOffset; bfi.bufferSize = VK_WHOLE_SIZE; createBufferRegion(info.bvhDataBuffer, bfi, vtAccelerator->_bvhBoxBuffer, _vtDevice); };

        // 
        if (!info.bvhMetaHeadBuffer) { 
            bfi.bufferSize = sizeof(VtBvhBlock) * 1ull;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhHeadingBuffer);
        } else 
        { bfi.offset = info.bvhMetaHeadOffset; bfi.bufferSize = VK_WHOLE_SIZE; createBufferRegion(info.bvhMetaHeadBuffer, bfi, vtAccelerator->_bvhHeadingBuffer, _vtDevice); };

        // 
        if (!info.bvhMetaBuffer) {  // create for backward compatibility 
            bfi.bufferSize = sizeof(VtBvhBlock) * 1ull;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhHeadingInBuffer);
        } else
        { bfi.offset = info.bvhMetaOffset; bfi.bufferSize = VK_WHOLE_SIZE; createBufferRegion(info.bvhMetaBuffer, bfi, vtAccelerator->_bvhHeadingInBuffer, _vtDevice); };

        // 
        if (!info.bvhInstanceBuffer) { // create for backward compatibility 
            bfi.bufferSize = sizeof(VtBvhInstance) * 1ull;
            createBufferRegion(bManager, bfi, vtAccelerator->_bvhInstancedBuffer);
        } else
        { bfi.offset = info.bvhInstanceOffset; bfi.bufferSize = VK_WHOLE_SIZE; createBufferRegion(info.bvhInstanceBuffer, bfi, vtAccelerator->_bvhInstancedBuffer, _vtDevice); };

        { // build final shared buffer for this class
            VtDeviceBufferCreateInfo bfic = {};
            bfic.familyIndex = _vtDevice->_mainFamilyIndex;
            bfic.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            createSharedBuffer(bManager, bfic, vtAccelerator->_sharedBuffer);
        };

        { // descriptor set (TODO: deprecate descriptor sets for bottom levels)
            std::vector<vk::PushConstantRange> constRanges = { vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2)) };
            std::vector<vk::DescriptorSetLayout> dsLayouts = { vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]) };

            // create descriptor set
            const auto&& dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_descriptorSet = std::move(dsc[0]);
        };

        { // 
            const auto writeTmpl = vk::WriteDescriptorSet(vtAccelerator->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)(&vtAccelerator->_bvhHeadingBuffer->_descriptorInfo())), // TODO: dedicated meta buffer
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)(&vtAccelerator->_bvhBoxBuffer->_descriptorInfo())),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)(&vtAccelerator->_bvhInstancedBuffer->_descriptorInfo())),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo((vk::DescriptorBufferInfo*)(&vtAccelerator->_bvhHeadingInBuffer->_descriptorInfo())),
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };

        return result;
    };


    //AcceleratorLinkedSet::~AcceleratorLinkedSet() {
    //    if (_descriptorSet) vk::Device(VkDevice(*_device)).freeDescriptorSets(_device->_descriptorPool, { vk::DescriptorSet(_descriptorSet) });
    //    _descriptorSet = {};
    //};
};
