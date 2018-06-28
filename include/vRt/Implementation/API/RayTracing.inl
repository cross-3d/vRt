#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;


    // planned type validations, also, planned advanced descriptor sets support in vertex assemblies
    VtResult bindDescriptorSets(std::shared_ptr<CommandBuffer>& cmdBuf, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet = 0, const std::vector<VkDescriptorSet>& descriptorSets = {}, const std::vector<VkDescriptorSet>& dynamicOffsets = {}) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_RAYTRACING) {
            cmdBuf->_boundDescriptorSets = descriptorSets;
        } else {
            cmdBuf->_boundVIDescriptorSets = descriptorSets;
        };
        return result;
    }

    // planned type validations
    VtResult bindPipeline(std::shared_ptr<CommandBuffer>& cmdBuf, VtPipelineBindPoint pipelineBindPoint, std::shared_ptr<Pipeline>& pipeline) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_RAYTRACING) {
            cmdBuf->_rayTracingPipeline = pipeline;
        };
        return result;
    }


    // update material data in command
    VtResult bindMaterialSet(std::shared_ptr<CommandBuffer>& cmdBuf, VtEntryUsageFlags usageIn, std::shared_ptr<MaterialSet> matrl) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_materialSet = matrl;
        return result;
    }


    VtResult dispatchRayTracing(std::shared_ptr<CommandBuffer>& cmdBuf, uint32_t x = 1, uint32_t y = 1, uint32_t B = 1) {
        VtResult result = VK_SUCCESS;

        auto device = cmdBuf->_parent();
        auto acclb = device->_acceleratorBuilder;
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();
        auto matrl = cmdBuf->_materialSet.lock();
        auto rtppl = cmdBuf->_rayTracingPipeline.lock();
        auto rtset = cmdBuf->_rayTracingSet.lock();

        auto rayCount = x * y;

        rtset->_cuniform.width = x;
        rtset->_cuniform.height = y;
        rtset->_cuniform.iteration = 0;
        rtset->_cuniform.closestHitOffset = 0;

        const uint32_t WG_COUNT = 64;
        const uint32_t RADICE_AFFINE = 16;
        const uint32_t uzero = 0u;

        // form descriptor sets
        std::vector<VkDescriptorSet> _rtSets = { rtset->_descriptorSet, matrl->_descriptorSet };
        for (auto &s : cmdBuf->_boundDescriptorSets) { _rtSets.push_back(s); }

        // reset counters of ray tracing
        cmdFillBuffer<0u>(*cmdBuf, *rtset->_countersBuffer);
        cmdFillBuffer<0u>(*cmdBuf, *rtset->_groupCountersBuffer);

        // initial consts
        vkCmdUpdateBuffer(*cmdBuf, *matrl->_constBuffer, 0, sizeof(uint32_t) * 2, &matrl->_materialCount);
        vkCmdUpdateBuffer(*cmdBuf, *rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);

        // run rays generation
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
        if (rtppl->_generationPipeline) cmdDispatch(*cmdBuf, rtppl->_generationPipeline, tiled(x, 8u), tiled(y, 8u));

        // ray trace command
        for (int it = 0; it < B; it++) {
            // update uniform buffer of ray tracing steps
            rtset->_cuniform.iteration = it;
            vkCmdUpdateBuffer(*cmdBuf, *rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform); 

            { // run traverse processing (single accelerator supported at now)
                std::vector<VkDescriptorSet> _tvSets = { rtset->_descriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
                vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_traversePipelineLayout, 0, _tvSets.size(), _tvSets.data(), 0, nullptr);
                cmdDispatch(*cmdBuf, acclb->_intersectionPipeline, INTENSIVITY); // traverse BVH
                cmdCopyBuffer(*cmdBuf, rtset->_countersBuffer, rtset->_constBuffer, { vk::BufferCopy(strided<uint32_t>(3), strided<uint32_t>(3), strided<uint32_t>(1)) });
                cmdDispatch(*cmdBuf, acclb->_interpolatorPipeline, INTENSIVITY); // interpolate intersections
            }

            // reload to caches and reset counters
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            cmdCopyBuffer(*cmdBuf, rtset->_groupCountersBuffer, rtset->_groupCountersBufferRead, { vk::BufferCopy(0, 0, 16 * sizeof(uint32_t)) });
            cmdCopyBuffer(*cmdBuf, rtset->_groupIndicesBuffer, rtset->_groupIndicesBufferRead, { vk::BufferCopy(0, 0, rayCount * sizeof(uint32_t) * 4) });
            cmdFillBuffer<0u>(*cmdBuf, *rtset->_countersBuffer);
            cmdFillBuffer<0u>(*cmdBuf, *rtset->_groupCountersBuffer);

            // handling hits in groups
            for (int i = 0; i < 4; i++) {
                rtset->_cuniform.rayGroup = i;
                vkCmdUpdateBuffer(*cmdBuf, *rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                if (rtppl->_closestHitPipeline[i]) cmdDispatch(*cmdBuf, rtppl->_closestHitPipeline[i], INTENSIVITY, 1, 1, false);
                if (rtppl->_missHitPipeline[i]) cmdDispatch(*cmdBuf, rtppl->_missHitPipeline[i], INTENSIVITY, 1, 1, false);
                if (rtppl->_missHitPipeline[i] || rtppl->_closestHitPipeline[i]) {
                    commandBarrier(*cmdBuf);
                }
            }

            // use resolve shader for resolve ray output or pushing secondaries
            for (int i = 0; i < 4; i++) {
                rtset->_cuniform.rayGroup = i;
                vkCmdUpdateBuffer(*cmdBuf, *rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                if (rtppl->_groupPipelines[i]) cmdDispatch(*cmdBuf, rtppl->_groupPipelines[i], INTENSIVITY);
            }
        }

        return result;
    }

};
