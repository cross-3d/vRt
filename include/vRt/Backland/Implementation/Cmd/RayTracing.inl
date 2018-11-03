#pragma once

//#include "../../vRt_subimpl.inl"
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
        auto acclb = device->_acceleratorBuilder[0];
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();
        auto matrl = cmdBuf->_materialSet.lock();
        auto rtppl = cmdBuf->_rayTracingPipeline.lock();
        auto rtset = cmdBuf->_rayTracingSet.lock();
        auto vasmp = vertx && vertx->_vertexAssembly ? vertx->_vertexAssembly : device->_nativeVertexAssembler[0];

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
        for (auto s : cmdBuf->_boundDescriptorSets) { _rtSets.push_back(s); }

        // cleaning indices and counters
        auto cmdClean = [&](){
            cmdFillBuffer<0u>(*cmdBuf, rtset->_countersBuffer);
            cmdFillBuffer<0u>(*cmdBuf, rtset->_groupCountersBuffer);
        };

        // run rays generation (if have)
        if (rtppl->_generationPipeline.size() > 0 && rtppl->_generationPipeline[0]) {
            cmdFillBuffer<0u>(*cmdBuf, rtset->_groupCountersBufferRead);
            cmdClean(), cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform), updateCommandBarrier(*cmdBuf);
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_rtLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            cmdDispatch(*cmdBuf, rtppl->_generationPipeline[0], tiled(x, rtppl->_tiling.width), tiled(y, rtppl->_tiling.height));
        };

        // ray trace command
        for (auto it = 0u; it < B; it++) {
            // update uniform buffer of ray tracing steps
            rtset->_cuniform.iteration = it;
            cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);

            // run traverse processing (single accelerator supported at now)
            if (vertx && vertx->_calculatedPrimitiveCount > 0) {
                if (vertx->_descriptorSetGenerator) vertx->_descriptorSetGenerator();
                if (accel->_descriptorSetGenerator) accel->_descriptorSetGenerator();

                // reset hit counter before new intersections
                auto zero = 0u; cmdUpdateBuffer(*cmdBuf, rtset->_countersBuffer, strided<uint32_t>(3), sizeof(uint32_t), &zero);
                cmdFillBuffer<-1>(*cmdBuf, rtset->_traverseCache);

                if (device->_hExtensionAccelerator.size() > 0 && device->_hExtensionAccelerator[0])
                { result = device->_hExtensionAccelerator[0]->_DoIntersections(cmdBuf, accel, rtset); } else // extension-based
                { // stock software BVH traverse (i.e. shader-based)
                    std::vector<VkDescriptorSet> _tvSets = { rtset->_descriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
                    vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_traversePipelineLayout, 0, _tvSets.size(), _tvSets.data(), 0, nullptr);
                    cmdDispatch(*cmdBuf, acclb->_intersectionPipeline, RV_INTENSIVITY); 
                };

                // interpolation hits
                if (vasmp->_intrpPipeline) {
                    std::vector<VkDescriptorSet> _tvSets = { rtset->_descriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
                    vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, vasmp->_pipelineLayout->_rtLayout, 0, _tvSets.size(), _tvSets.data(), 0, nullptr);
                    cmdDispatch(*cmdBuf, vasmp->_intrpPipeline, RV_INTENSIVITY); // interpolate intersections
                };

                // multiple-time traversing no more needed since added instancing 
                //cmdCopyBuffer(*cmdBuf, rtset->_countersBuffer, rtset->_constBuffer, { vk::BufferCopy(strided<uint32_t>(3), offsetof(VtStageUniform, closestHitOffset), sizeof(uint32_t)) });
                //cmdUpdateBuffer(*cmdBuf, rtset->_countersBuffer, strided<uint32_t>(1), sizeof(uint32_t), &zero); //reset collection hit counter
            }

            // reload to caches and reset counters (if has group shaders)
            bool hasGroupShaders = false;
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, rtppl->_pipelineLayout->_rtLayout, 0, _rtSets.size(), _rtSets.data(), 0, nullptr);
            for (int i = 0; i < std::min(std::size_t(4ull), rtppl->_groupPipeline.size()); i++) {
                if (rtppl->_groupPipeline[i]) {
                    cmdCopyBuffer(*cmdBuf, rtset->_groupCountersBuffer, rtset->_groupCountersBufferRead, { vk::BufferCopy(0, 0, 64ull * sizeof(uint32_t)) });
                    cmdCopyBuffer(*cmdBuf, rtset->_groupIndicesBuffer, rtset->_groupIndicesBufferRead, { vk::BufferCopy(0, 0, rayCount * MAX_RAY_GROUPS * sizeof(uint32_t)) });
                    updateCommandBarrier(*cmdBuf), cmdClean();
                    hasGroupShaders = true; break;
                }
            }
            



            // handling hits in groups
            for (int i = 0; i < std::min(std::size_t(4ull), rtppl->_closestHitPipeline.size()); i++) {
                if (rtppl->_closestHitPipeline[i]) {
                    rtset->_cuniform.currentGroup = i;
                    cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform), updateCommandBarrier(*cmdBuf);
                    cmdDispatch(*cmdBuf, rtppl->_closestHitPipeline[i], INTENSIVITY);
                }
            }

            // handling misses in groups
            // moved to after in 11.10.2018
            if (rtppl->_missHitPipeline[0]) {
                cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                cmdDispatch(*cmdBuf, rtppl->_missHitPipeline[0], INTENSIVITY);
            }

            // clear counters for pushing newer data
            //if (hasGroupShaders) cmdFillBuffer<0u>(*cmdBuf, rtset->_countersBuffer);

            // use resolve shader for resolve ray output or pushing secondaries
            for (auto i = 0u; i < std::min(std::size_t(4ull), rtppl->_groupPipeline.size()); i++) {
                if (rtppl->_groupPipeline[i]) {
                    rtset->_cuniform.currentGroup = i;
                    cmdUpdateBuffer(*cmdBuf, rtset->_constBuffer, 0, sizeof(rtset->_cuniform), &rtset->_cuniform);
                    cmdDispatch(*cmdBuf, rtppl->_groupPipeline[i], INTENSIVITY);
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
