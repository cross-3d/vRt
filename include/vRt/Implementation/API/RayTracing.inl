#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;


    VtResult bindPipeline(std::shared_ptr<CommandBuffer>& cmdBuf, VtPipelineBindPoint pipelineBindPoint, std::shared_ptr<Pipeline>& pipeline) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_RAY_TRACING) {
            cmdBuf->_rayTracingPipeline = pipeline;
        };
        return result;
    }

    VtResult dispatchRayTracing(std::shared_ptr<CommandBuffer>& cmdBuf, uint32_t x = 1, uint32_t y = 1) {
        VtResult result = VK_SUCCESS;

        auto &device = cmdBuf->_parent();
        auto &acclb = device->_acceleratorBuilder;
        auto &accel = cmdBuf->_acceleratorSet;
        auto &vertx = cmdBuf->_vertexSet;
        auto &matrl = cmdBuf->_materialSet;
        auto &rtppl = cmdBuf->_rayTracingPipeline;
        auto &rtset = cmdBuf->_rayTracingSet;

        const uint32_t WG_COUNT = 64;
        const uint32_t RADICE_AFFINE = 16;

        std::vector<VkDescriptorSet> _rtSets = { rtset->_descriptorSet, matrl->_descriptorSet };
        std::vector<VkDescriptorSet> _tvSets = { rtset->_descriptorSet, accel->_descriptorSet, vertx->_descriptorSet };

        // bind user descriptor sets
        for (auto &s: cmdBuf->_boundDescriptorSets) {
            _rtSets.push_back(s);
        }


        {
            // reset counters of ray tracing
            cmdFillBuffer<0u>(*cmdBuf, *rtset->_countersBuffer);

            // upload constants to material sets
            vkCmdUpdateBuffer(*cmdBuf, *matrl->_constBuffer, 0, sizeof(uint32_t) * 2, &matrl->_materialCount);
            fromHostCommandBarrier(*cmdBuf);

            // run rays generation
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            if (rtppl->_generationPipeline) cmdDispatch(*cmdBuf, rtppl->_generationPipeline, tiled(x, 8u), tiled(y, 8u));

            // run traverse processing 
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_traversePipelineLayout, 0, _tvSets.size(), _tvSets.data(), 0, nullptr);
            cmdDispatch(*cmdBuf, acclb->_intersectionPipeline, 4096);

            // handling hits
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            if (rtppl->_closestHitPipeline) cmdDispatch(*cmdBuf, rtppl->_closestHitPipeline, 4096);
            if (rtppl->_missHitPipeline) cmdDispatch(*cmdBuf, rtppl->_missHitPipeline, 4096);
            if (rtppl->_resolvePipeline) cmdDispatch(*cmdBuf, rtppl->_resolvePipeline, 4096);
        }

        return result;
    }

};
