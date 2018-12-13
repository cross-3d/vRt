#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;

    // planned add hardcodes for radix sorting
    VtResult createRadixSort(std::shared_ptr<Device> device, VtDeviceAggregationInfo vtExtension, std::shared_ptr<RadixSort>& vtRadix) {
        //constexpr const auto STEPS = VRT_USE_MORTON_32 ? 4ull : 8ull, WG_COUNT = 128ull, RADICE_AFFINE = 256ull; // 8-bit (NOT effective in RX Vega)
        //constexpr const auto STEPS = VRT_USE_MORTON_32 ? 8ull : 16ull, WG_COUNT = 128ull, RADICE_AFFINE = 16ull; // QLC
        //constexpr const auto STEPS = VRT_USE_MORTON_32 ? 16ull : 32ull, WG_COUNT = 128ull, RADICE_AFFINE = 4ull; // MLC


        const auto vendorName = device->_vendorName;
        auto STEPS = VRT_USE_MORTON_32 ? 8ull : 16ull, WG_COUNT = 64ull, RADICE_AFFINE = 16ull;
        if (vendorName == VT_VENDOR_NV_TURING) { STEPS = VRT_USE_MORTON_32 ? 4ull : 8ull, WG_COUNT = 64ull, RADICE_AFFINE = 256ull; };


        VtResult result = VK_SUCCESS;
        auto vkPipelineCache = device->_pipelineCache;
        vtRadix = std::make_shared<RadixSort>();
        vtRadix->_device = device;

        
        std::shared_ptr<BufferManager> bManager = {};
        createBufferManager(device, bManager);

        VtDeviceBufferCreateInfo bfic = {};
        bfic.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VtBufferRegionCreateInfo bfi = {};
        {
            bfi.format = VK_FORMAT_R32_UINT;
            bfi.bufferSize = vtExtension.maxPrimitives * sizeof(uint64_t);
            createBufferRegion(bManager, bfi, vtRadix->_tmpValuesBuffer);

            bfi.format = VK_FORMAT_R32G32_UINT;
            bfi.bufferSize = vtExtension.maxPrimitives * sizeof(uint64_t);
            createBufferRegion(bManager, bfi, vtRadix->_tmpKeysBuffer);

            bfi.format = VK_FORMAT_UNDEFINED;
            bfi.bufferSize = 16ull * STEPS;
            createBufferRegion(bManager, bfi, vtRadix->_stepsBuffer); // unused

            bfi.format = VK_FORMAT_R32_UINT;
            bfi.bufferSize = RADICE_AFFINE * WG_COUNT * sizeof(uint32_t);
            createBufferRegion(bManager, bfi, vtRadix->_histogramBuffer);
            createBufferRegion(bManager, bfi, vtRadix->_prefixSumBuffer);
        }

        { // build final shared buffer for this class
            createSharedBuffer(bManager, bfic, vtRadix->_sharedBuffer);
        }


        std::vector<vk::PushConstantRange> constRanges = {
            vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
        };

        std::vector<vk::DescriptorSetLayout> dsLayouts = {
            vk::DescriptorSetLayout(device->_descriptorLayoutMap["radixSort"]),
            vk::DescriptorSetLayout(device->_descriptorLayoutMap["radixSortBind"])
        };

        auto vkDevice = device->_device;
        vtRadix->_pipelineLayout = vk::Device(vkDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
        vtRadix->_histogramPipeline = createComputeHC(vkDevice, getCorrectPath(qradix::histogram, vendorName, device->_shadersPath), vtRadix->_pipelineLayout, vkPipelineCache);
        vtRadix->_workPrefixPipeline = createComputeHC(vkDevice, getCorrectPath(qradix::workPrefix, vendorName, device->_shadersPath), vtRadix->_pipelineLayout, vkPipelineCache);
        vtRadix->_permutePipeline = createComputeHC(vkDevice, getCorrectPath(qradix::permute, vendorName, device->_shadersPath), vtRadix->_pipelineLayout, vkPipelineCache);
        vtRadix->_copyhackPipeline = createComputeHC(vkDevice, getCorrectPath(qradix::copyhack, vendorName, device->_shadersPath), vtRadix->_pipelineLayout, vkPipelineCache);

        const auto&& dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(device->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
        vtRadix->_descriptorSet = std::move(dsc[0]);

        // write radix sort descriptor sets
        const auto writeTmpl = vk::WriteDescriptorSet(vtRadix->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRadix->_tmpKeysBuffer->_descriptorInfo()),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRadix->_tmpValuesBuffer->_descriptorInfo()),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRadix->_stepsBuffer->_descriptorInfo()), //unused
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRadix->_histogramBuffer->_descriptorInfo()),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRadix->_prefixSumBuffer->_descriptorInfo()),
        };
        vk::Device(vkDevice).updateDescriptorSets(writes, {});

        return result;
    };

};
