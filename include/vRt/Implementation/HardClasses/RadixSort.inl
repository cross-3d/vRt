#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // planned add hardcodes for radix sorting
    inline VtResult createRadixSort(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& vtExtension, std::shared_ptr<RadixSort>& _vtRadix) {
        VtResult result = VK_SUCCESS;

        auto& vtRadix = (_vtRadix = std::make_shared<RadixSort>());
        vtRadix->_device = _vtDevice;

        //constexpr auto STEPS = RVT_USE_MORTON_32 ? 4ull : 8ull, WG_COUNT = 64ull, RADICE_AFFINE = 256ull; // 8-bit
        constexpr auto STEPS = RVT_USE_MORTON_32 ? 8ull : 16ull, WG_COUNT = 64ull, RADICE_AFFINE = 16ull; // QLC
        //constexpr auto STEPS = RVT_USE_MORTON_32 ? 16ull : 32ull, WG_COUNT = 64ull, RADICE_AFFINE = 4ull; // MLC

        const auto& vendorName = _vtDevice->_vendorName;

        VtDeviceBufferCreateInfo bfi;
        bfi.familyIndex = _vtDevice->_mainFamilyIndex;
        bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

        bfi.format = VK_FORMAT_R32_UINT;
        bfi.bufferSize = vtExtension.maxPrimitives * sizeof(uint32_t);
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_tmpValuesBuffer);

        bfi.format = VK_FORMAT_R32G32_UINT;
        bfi.bufferSize = vtExtension.maxPrimitives * sizeof(uint64_t);
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_tmpKeysBuffer);

        bfi.format = VK_FORMAT_UNDEFINED;
        bfi.bufferSize = 16ull * STEPS;
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_stepsBuffer); // unused

        bfi.format = VK_FORMAT_R32_UINT;
        bfi.bufferSize = RADICE_AFFINE * WG_COUNT * sizeof(uint32_t);
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
        vtRadix->_histogramPipeline = createComputeMemory(VkDevice(*_vtDevice), qradix::histogram[vendorName], vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_workPrefixPipeline = createComputeMemory(VkDevice(*_vtDevice), qradix::workPrefix[vendorName], vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_permutePipeline = createComputeMemory(VkDevice(*_vtDevice), qradix::permute[vendorName], vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_copyhackPipeline = createComputeMemory(VkDevice(*_vtDevice), qradix::copyhack[vendorName], vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));

        auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
        vtRadix->_descriptorSet = dsc[0];

        // write radix sort descriptor sets
        vk::WriteDescriptorSet _write_tmpl = vk::WriteDescriptorSet(vtRadix->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_tmpKeysBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_tmpValuesBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_stepsBuffer->_descriptorInfo())), //unused
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_histogramBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_prefixSumBuffer->_descriptorInfo())),
        };
        vk::Device(*_vtDevice).updateDescriptorSets(writes, {});

        return result;
    };

};
