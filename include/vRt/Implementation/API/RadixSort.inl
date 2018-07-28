#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // radix sorting command (qRadix)
    VtResult radixSort(std::shared_ptr<CommandBuffer>& cmdBuf, const VkDescriptorSet& inputSet, uint32_t primCount = 2) {
        constexpr auto STEPS = RVT_USE_MORTON_32 ? 4ull : 8ull, WG_COUNT = 64ull, RADICE_AFFINE = 1ull;
        //constexpr auto STEPS = RVT_USE_MORTON_32 ? 8ull : 16ull, WG_COUNT = 64ull, RADICE_AFFINE = 1ull; // QLC
        //constexpr auto STEPS = RVT_USE_MORTON_32 ? 16ull : 32ull, WG_COUNT = 64ull, RADICE_AFFINE = 1ull; // MLC


        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        auto radix = device->_radixSort;
        std::vector<VkDescriptorSet> _sets = { radix->_descriptorSet, inputSet };
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, radix->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        //updateCommandBarrier(*cmdBuf);
        for (uint32_t i = 0; i < STEPS; i++) {
            std::vector<uint32_t> _values = { primCount, i };
            vkCmdPushConstants(*cmdBuf, radix->_pipelineLayout, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * _values.size(), _values.data());
            
            cmdDispatch(*cmdBuf, radix->_histogramPipeline, WG_COUNT, RADICE_AFFINE);
            cmdDispatch(*cmdBuf, radix->_workPrefixPipeline);
            cmdDispatch(*cmdBuf, radix->_permutePipeline, WG_COUNT, RADICE_AFFINE);
            cmdDispatch(*cmdBuf, radix->_copyhackPipeline, INTENSIVITY);
        }
        return result;
    }

};
