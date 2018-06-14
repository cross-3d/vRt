#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // radix sorting command (qRadix)
    VtResult radixSort(std::shared_ptr<CommandBuffer>& cmdBuf, const VkDescriptorSet& inputSet, uint32_t primCount = 2) {
        constexpr auto STEPS = 8u, WG_COUNT = 64u, RADICE_AFFINE = 16u;

        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        auto radix = device->_radixSort;
        std::vector<VkDescriptorSet> _sets = { radix->_descriptorSet, inputSet };
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, radix->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        for (uint32_t i = 0; i < STEPS; i++) {
            const uint32_t _values[2] = { primCount, i };
            vkCmdPushConstants(*cmdBuf, radix->_pipelineLayout, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t) * 2, _values);
            cmdDispatch(*cmdBuf, radix->_histogramPipeline, WG_COUNT, RADICE_AFFINE);
            cmdDispatch(*cmdBuf, radix->_workPrefixPipeline);
            cmdDispatch(*cmdBuf, radix->_permutePipeline, WG_COUNT, RADICE_AFFINE);
        }
        return result;
    }

};
