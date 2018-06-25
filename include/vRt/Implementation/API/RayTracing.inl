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


    VtResult dispatchRayTracing(std::shared_ptr<CommandBuffer>& cmdBuf, uint32_t x = 1, uint32_t y = 1) {
        VtResult result = VK_SUCCESS;

        auto device = cmdBuf->_parent();
        auto acclb = device->_acceleratorBuilder;
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();
        auto matrl = cmdBuf->_materialSet.lock();
        auto rtppl = cmdBuf->_rayTracingPipeline.lock();
        auto rtset = cmdBuf->_rayTracingSet.lock();

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

        // update material set
        vkCmdUpdateBuffer(*cmdBuf, *matrl->_constBuffer, 0, sizeof(uint32_t) * 2, &matrl->_materialCount);

        // ray trace command
        for (int i = 0; i < 1; i++) { // TODO make support of steps
            rtset->_cuniform.iteration = i;

            // update uniform buffer of ray tracing steps
            vkCmdUpdateBuffer(*cmdBuf, *rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform); 
            commandBarrier(*cmdBuf);

            // run rays generation
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            if (rtppl->_generationPipeline) cmdDispatch(*cmdBuf, rtppl->_generationPipeline, tiled(x, 8u), tiled(y, 8u));

            { // run traverse processing
                std::vector<VkDescriptorSet> _tvSets = { rtset->_descriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
                vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_traversePipelineLayout, 0, _tvSets.size(), _tvSets.data(), 0, nullptr);
                cmdDispatch(*cmdBuf, acclb->_intersectionPipeline, INTENSIVITY); // traverse BVH
                cmdCopyBuffer(*cmdBuf, rtset->_countersBuffer, rtset->_constBuffer, { vk::BufferCopy(strided<uint32_t>(3), strided<uint32_t>(3), strided<uint32_t>(1)) });
                cmdDispatch(*cmdBuf, acclb->_interpolatorPipeline, INTENSIVITY); // interpolate intersections
                vkCmdUpdateBuffer(*cmdBuf, *rtset->_countersBuffer, strided<uint32_t>(2), sizeof(uint32_t), &uzero);
                commandBarrier(*cmdBuf);
            }

            // handling hits
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            if (rtppl->_closestHitPipeline) cmdDispatch(*cmdBuf, rtppl->_closestHitPipeline, INTENSIVITY);
            if (rtppl->_missHitPipeline) cmdDispatch(*cmdBuf, rtppl->_missHitPipeline, INTENSIVITY);

            for (int i = 0; i < 4; i++) {
                rtset->_cuniform.rayGroup = i;
                vkCmdUpdateBuffer(*cmdBuf, *rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                commandBarrier(*cmdBuf);

                if (rtppl->_resolvePipelines[i]) cmdDispatch(*cmdBuf, rtppl->_resolvePipelines[i], INTENSIVITY);
            }
        }

        return result;
    }

};
