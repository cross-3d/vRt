#pragma once

#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"


namespace _vt {
    using namespace vrt;


    // planned type validations, also, planned advanced descriptor sets support in vertex assemblies
    VtResult bindDescriptorSets(std::shared_ptr<CommandBuffer> cmdBuf, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t firstSet = 0, const std::vector<VkDescriptorSet>& descriptorSets = {}, const std::vector<VkDescriptorSet>& dynamicOffsets = {}) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_RAYTRACING) {
            cmdBuf->_boundDescriptorSets = descriptorSets;
        } else {
            cmdBuf->_boundVIDescriptorSets = descriptorSets;
        };
        return result;
    };

    // planned type validations
    VtResult bindPipeline(std::shared_ptr<CommandBuffer> cmdBuf, VtPipelineBindPoint pipelineBindPoint, std::shared_ptr<Pipeline> pipeline) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_RAYTRACING) {
            cmdBuf->_rayTracingPipeline = pipeline;
        };
        return result;
    };

    // update material data in command
    VtResult bindMaterialSet(std::shared_ptr<CommandBuffer> cmdBuf, VtEntryUsageFlags usageIn, std::shared_ptr<MaterialSet> matrl) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_materialSet = matrl;
        return result;
    };

    // ray-tracing pipeline 
    VtResult dispatchRayTracing(std::shared_ptr<CommandBuffer> cmdBuf, uint32_t x = 1, uint32_t y = 1, uint32_t B = 1) {
        constexpr const auto WG_COUNT = 64u, RADICE_AFFINE = 16u;

        VtResult result = VK_SUCCESS;

        auto device = cmdBuf->_parent();
        auto acclb = device->_acceleratorBuilder;
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();
        auto matrl = cmdBuf->_materialSet.lock();
        auto rtppl = cmdBuf->_rayTracingPipeline.lock();
        auto rtset = cmdBuf->_rayTracingSet.lock();

        const auto rayCount = x * y;
        rtset->_cuniform.width  = x;
        rtset->_cuniform.height = y;
        rtset->_cuniform.iteration = 0;
        rtset->_cuniform.closestHitOffset = 0;
        rtset->_cuniform.currentGroup = 0;
        rtset->_cuniform.maxRayCount = rayCount; // try to
        rtset->_cuniform.lastIteration = B-1;

        // form descriptor sets
        std::vector<VkDescriptorSet> _rtSets = { rtset->_descriptorSet };
        if (matrl) {
            _rtSets.push_back(matrl->_descriptorSet); // make material set not necesssary
            cmdUpdateBuffer(*cmdBuf, matrl->_constBuffer, 0, sizeof(uint32_t) * 2, &matrl->_materialCount);
        };
        for (auto &s : cmdBuf->_boundDescriptorSets) { _rtSets.push_back(s); }

        // cleaning indices and counters
        auto cmdClean = [&](){
            cmdFillBuffer<0u>(*cmdBuf, rtset->_countersBuffer);
            cmdFillBuffer<0u>(*cmdBuf, rtset->_groupCountersBuffer);
            //cmdFillBuffer<0u>(*cmdBuf, rtset->_groupIndicesBuffer);
            updateCommandBarrier(*cmdBuf);
        };

        // run rays generation (if have)
        if (rtppl->_generationPipeline.size() > 0 && rtppl->_generationPipeline[0]) {
            cmdClean(); cmdFillBuffer<0u>(*cmdBuf, rtset->_rayLinkPayload);
            cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            cmdDispatch(*cmdBuf, rtppl->_generationPipeline[0], tiled(x, rtppl->_tiling.width), tiled(y, rtppl->_tiling.height));
        };

        // ray trace command
        for (uint32_t it = 0; it < B; it++) {
            // update uniform buffer of ray tracing steps
            rtset->_cuniform.iteration = it;
            cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);

            // run traverse processing (single accelerator supported at now)
            if (vertx && vertx->_calculatedPrimitiveCount > 0) {
                auto zero = 0u; cmdUpdateBuffer(*cmdBuf, rtset->_countersBuffer, strided<uint32_t>(3), sizeof(uint32_t), &zero);
                if (device->_advancedAccelerator) {
                    result = device->_advancedAccelerator->_DoIntersections(cmdBuf, accel, rtset);
                }
                else {
                    std::vector<VkDescriptorSet> _tvSets = { rtset->_descriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
                    vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_traversePipelineLayout, 0, _tvSets.size(), _tvSets.data(), 0, nullptr);

                    // reset hit counter before new intersections
                    cmdDispatch(*cmdBuf, acclb->_intersectionPipeline, RV_INTENSIVITY); // traverse BVH
                    cmdDispatch(*cmdBuf, acclb->_interpolatorPipeline, RV_INTENSIVITY); // interpolate intersections
                };
                cmdCopyBuffer(*cmdBuf, rtset->_countersBuffer, rtset->_constBuffer, { vk::BufferCopy(strided<uint32_t>(3), offsetof(VtStageUniform, closestHitOffset), strided<uint32_t>(1)) });
            }

            // reload to caches and reset counters (if has group shaders)
            bool hasGroupShaders = false;
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_pipelineLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            for (int i = 0; i < std::min(std::size_t(4ull), rtppl->_groupPipelines.size()); i++) {
                if (rtppl->_groupPipelines[i]) {
                    cmdCopyBuffer(*cmdBuf, rtset->_groupCountersBuffer, rtset->_groupCountersBufferRead, { vk::BufferCopy(0, 0, 16 * sizeof(uint32_t)) });
                    cmdCopyBuffer(*cmdBuf, rtset->_groupIndicesBuffer, rtset->_groupIndicesBufferRead, { vk::BufferCopy(0, 0, rayCount * (4ull+1ull) * sizeof(uint32_t)) });
                    cmdClean();
                    hasGroupShaders = true; break;
                }
            }
            
            // handling misses in groups
            if (rtppl->_missHitPipeline[0]) {
                cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                cmdDispatch(*cmdBuf, rtppl->_missHitPipeline[0], INTENSIVITY);
            }

            // handling hits in groups
            for (int i = 0; i < std::min(std::size_t(4ull), rtppl->_closestHitPipeline.size()); i++) {
                if (rtppl->_closestHitPipeline[i]) {
                    rtset->_cuniform.currentGroup = i;
                    cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                    cmdDispatch(*cmdBuf, rtppl->_closestHitPipeline[i], INTENSIVITY);
                }
            }

            // clear counters for pushing newer data
            if (hasGroupShaders) cmdFillBuffer<0u>(*cmdBuf, rtset->_countersBuffer);

            // use resolve shader for resolve ray output or pushing secondaries
            for (int i = 0; i < std::min(std::size_t(4ull), rtppl->_groupPipelines.size()); i++) {
                if (rtppl->_groupPipelines[i]) {
                    rtset->_cuniform.currentGroup = i;
                    cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                    cmdDispatch(*cmdBuf, rtppl->_groupPipelines[i], INTENSIVITY);
                }
            }
        }

        return result;
    };

    // create wrapped command buffer
    VtResult queryCommandBuffer(std::shared_ptr<Device> _vtDevice, VkCommandBuffer cmdBuf, std::shared_ptr<CommandBuffer>& vtCmdBuf) {
        vtCmdBuf = std::make_shared<CommandBuffer>();
        vtCmdBuf->_device = _vtDevice;
        vtCmdBuf->_commandBuffer = cmdBuf;
        return VK_SUCCESS;
    };

};
