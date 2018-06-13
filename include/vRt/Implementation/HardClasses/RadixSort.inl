#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;


    inline VtResult createRadixSort(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& vtExtension, std::shared_ptr<RadixSort>& _vtRadix) {
        auto& vtRadix = (_vtRadix = std::make_shared<RadixSort>());
        vtRadix->_device = _vtDevice;

        constexpr auto STEPS = 8u, WG_COUNT = 64u, RADICE_AFFINE = 16u;

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
        bfi.bufferSize = 16 * STEPS;
        createDeviceBuffer(_vtDevice, bfi, vtRadix->_stepsBuffer); // unused

        bfi.format = VK_FORMAT_R32_UINT;
        bfi.bufferSize = 1024 * RADICE_AFFINE * sizeof(uint32_t);
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
        vtRadix->_histogramPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "qRadix/histogram.comp.spv", vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_workPrefixPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "qRadix/pfx-work.comp.spv", vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtRadix->_permutePipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "qRadix/permute.comp.spv", vtRadix->_pipelineLayout, VkPipelineCache(*_vtDevice));

        auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
        vtRadix->_descriptorSet = dsc[0];

        // write radix sort descriptor sets
        vk::WriteDescriptorSet _write_tmpl = vk::WriteDescriptorSet(vtRadix->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_stepsBuffer->_descriptorInfo())), //unused
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_tmpKeysBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_tmpValuesBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_histogramBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setPBufferInfo(&vk::DescriptorBufferInfo(vtRadix->_prefixSumBuffer->_descriptorInfo())),
        };
        vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
    };

};
