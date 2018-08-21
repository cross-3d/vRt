#pragma once

#include "../../vRt_subimpl.inl"
#include "RadixSort.inl"

namespace _vt {
    using namespace vrt;

    VtResult bindDescriptorSetsPerVertexInput(std::shared_ptr<CommandBuffer> cmdBuf, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t vertexInputID = 0, uint32_t firstSet = 0, const std::vector<VkDescriptorSet>& descriptorSets = {}, const std::vector<VkDescriptorSet>& dynamicOffsets = {}) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_VERTEXASSEMBLY) {
            cmdBuf->_perVertexInputDSC[vertexInputID] = descriptorSets;
        };
        return result;
    }

    VtResult bindVertexInputs(std::shared_ptr<CommandBuffer> cmdBuf, const std::vector<std::shared_ptr<VertexInputSet>>& sets) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexInputs.resize(0);
        for (auto s : sets) { // update buffers by pushing constants
            cmdBuf->_vertexInputs.push_back(s);
        };
        return result;
    }


    VtResult bindAccelerator(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<AcceleratorSet>& accSet) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_acceleratorSet = accSet;
        return result;
    }

    // bind vertex assembly (also, do imageBarrier)
    VtResult bindVertexAssembly(std::shared_ptr<CommandBuffer> cmdBuf, std::shared_ptr<VertexAssemblySet>& vasSet) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexSet = vasSet;
        return result;
    }

    VtResult cmdVertexAssemblyBarrier(VkCommandBuffer cmdBuf, std::shared_ptr<VertexAssemblySet>& vasSet) {
        VtResult result = VK_SUCCESS;
        imageBarrier(cmdBuf, vasSet->_attributeTexelBuffer);
        return result;
    }

    VtResult cmdBarrierAggregated(std::shared_ptr<CommandBuffer> cmdBuf) {
        auto device = cmdBuf->_parent();
        cmdDispatch(*cmdBuf, device->_dullBarrier);
        return VK_SUCCESS;
    }

    VtResult buildVertexSet(std::shared_ptr<CommandBuffer> cmdBuf, bool useInstance = true, std::function<void(VkCommandBuffer, int, VtUniformBlock&)> cb = {}) {
        VtResult result = VK_SUCCESS;

        // useless to building
        if (cmdBuf->_vertexInputs.size() <= 0) return result;

        auto device = cmdBuf->_parent();
        auto vertbd = device->_vertexAssembler;
        auto vertx = cmdBuf->_vertexSet.lock();

        // update constants
        imageBarrier(*cmdBuf, vertx->_attributeTexelBuffer);
        uint32_t _bndc = 0, calculatedPrimitiveCount = 0;
        for (auto iV_ : cmdBuf->_vertexInputs) {
            uint32_t _bnd = _bndc++;
            auto iV = iV_.lock();

            //iV->_uniformBlock.updateOnly = false;
            iV->_uniformBlock.primitiveOffset = calculatedPrimitiveCount;
            if (cb) cb(*cmdBuf, int(_bnd), iV->_uniformBlock);
            cmdUpdateBuffer(*cmdBuf, iV->_uniformBlockBuffer, strided<VtUniformBlock>(_bnd), sizeof(iV->_uniformBlock), &iV->_uniformBlock);
            calculatedPrimitiveCount += iV->_uniformBlock.primitiveCount;
        } _bndc = 0;
        updateCommandBarrier(*cmdBuf);

        vertx->_calculatedPrimitiveCount = calculatedPrimitiveCount;
        if (useInstance) {
            uint32_t _bnd = 0, _szi = cmdBuf->_vertexInputs.size();
            auto iV = cmdBuf->_vertexInputs[_bnd].lock();

            // native descriptor sets
            auto vertb = iV->_vertexAssembly ? iV->_vertexAssembly : vertbd;
            std::vector<VkDescriptorSet> _sets = { vertx->_descriptorSet, iV->_descriptorSet };

            // user defined descriptor sets
            auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
            for (auto &s : _bsets) { _sets.push_back(s); }

            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *vertb->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr); // bind descriptor sets
            vkCmdPushConstants(*cmdBuf, *vertb->_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &_bnd);
            cmdDispatch(*cmdBuf, vertb->_vertexAssemblyPipeline, INTENSIVITY, _szi, 1);
        } else {
            for (auto iV_ : cmdBuf->_vertexInputs) {
                uint32_t _bnd = _bndc++;
                auto iV = iV_.lock();

                // native descriptor sets
                auto vertb = iV->_vertexAssembly ? iV->_vertexAssembly : vertbd;
                std::vector<VkDescriptorSet> _sets = { vertx->_descriptorSet, iV->_descriptorSet };

                // user defined descriptor sets
                auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
                for (auto &s : _bsets) { _sets.push_back(s); }

                vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *vertb->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr); // bind descriptor sets
                vkCmdPushConstants(*cmdBuf, *vertb->_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &_bnd);
                cmdDispatch(*cmdBuf, vertb->_vertexAssemblyPipeline, INTENSIVITY, 1, 1, false);
            }
            commandBarrier(*cmdBuf);
        }
        return result;
    }

    // update region of vertex set by bound input set
    VtResult updateVertexSet(std::shared_ptr<CommandBuffer> cmdBuf, uint32_t inputSet = 0, bool multiple = false, bool useInstance = true, std::function<void(VkCommandBuffer, int, VtUniformBlock&)> cb = {}) {
        VtResult result = VK_SUCCESS;

        // useless to updating
        if (cmdBuf->_vertexInputs.size() <= 0) return result;

        auto device = cmdBuf->_parent();
        auto vertbd = device->_vertexAssembler;
        auto vertx = cmdBuf->_vertexSet.lock();

        // update constants
        uint32_t _bndc = 0, calculatedPrimitiveCount = 0;
        for (auto iV_ : cmdBuf->_vertexInputs) {
            uint32_t _bnd = _bndc++;
            auto iV = iV_.lock();

            //iV->_uniformBlock.updateOnly = true;
            iV->_uniformBlock.primitiveOffset = calculatedPrimitiveCount;
            if (cb) cb(*cmdBuf, int(_bnd), iV->_uniformBlock);
            cmdUpdateBuffer(*cmdBuf, iV->_uniformBlockBuffer, strided<VtUniformBlock>(_bnd), sizeof(iV->_uniformBlock), &iV->_uniformBlock);
            calculatedPrimitiveCount += iV->_uniformBlock.primitiveCount;
        } _bndc = 0;
        updateCommandBarrier(*cmdBuf);
        
        if (useInstance || !multiple) {
            uint32_t _bnd = inputSet;
            uint32_t _szi = cmdBuf->_vertexInputs.size() - inputSet;
            auto iV = cmdBuf->_vertexInputs[_bnd].lock();

            // native descriptor sets
            auto vertb = iV->_vertexAssembly ? iV->_vertexAssembly : vertbd;
            std::vector<VkDescriptorSet> _sets = { vertx->_descriptorSet, iV->_descriptorSet };

            // user defined descriptor sets
            auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
            for (auto &s : _bsets) { _sets.push_back(s); }

            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *vertb->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr); // bind descriptor sets
            vkCmdPushConstants(*cmdBuf, *vertb->_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &_bnd);
            cmdDispatch(*cmdBuf, vertb->_vertexAssemblyPipeline, INTENSIVITY, multiple ? _szi : 1, 1);
        } else {
            for (auto iV_ : cmdBuf->_vertexInputs) {
                uint32_t _bnd = _bndc++;
                if (_bnd >= inputSet) {
                    auto iV = iV_.lock();

                    // native descriptor sets
                    auto vertb = iV->_vertexAssembly ? iV->_vertexAssembly : vertbd;
                    std::vector<VkDescriptorSet> _sets = { vertx->_descriptorSet, iV->_descriptorSet };

                    // user defined descriptor sets
                    auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
                    for (auto &s : _bsets) { _sets.push_back(s); }

                    vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *vertb->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr); // bind descriptor sets
                    vkCmdPushConstants(*cmdBuf, *vertb->_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &_bnd);
                    cmdDispatch(*cmdBuf, vertb->_vertexAssemblyPipeline, INTENSIVITY, 1, 1, false);
                }
            }
            commandBarrier(*cmdBuf);
        }

        
        return result;
    }

    VtResult buildAccelerator(std::shared_ptr<CommandBuffer> cmdBuf) {
        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        auto acclb = device->_acceleratorBuilder;
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();
        const VtMat4 initialMat = {
            { 1.f, 0.f, 0.f, 0.f },
            { 0.f, 1.f, 0.f, 0.f },
            { 0.f, 0.f, 1.f, 0.f },
            { 0.f, 0.f, 0.f, 1.f },
        };

        accel->_bvhBlockData.primitiveOffset = accel->_primitiveOffset;
        accel->_bvhBlockData.primitiveCount = (accel->_primitiveCount != -1 && accel->_primitiveCount >= 0) ? accel->_primitiveCount : vertx->_calculatedPrimitiveCount;
        accel->_bvhBlockData.leafCount = accel->_bvhBlockData.primitiveCount;
        accel->_bvhBlockData.entryID = accel->_entryID;
        accel->_bvhBlockData.projection = initialMat;
        accel->_bvhBlockData.projectionInv = initialMat;
        accel->_bvhBlockData.transform = initialMat;
        accel->_bvhBlockData.transformInv = initialMat;
        cmdUpdateBuffer(*cmdBuf, accel->_bvhBlockUniform, 0, sizeof(accel->_bvhBlockData), &accel->_bvhBlockData);

        // building hlBVH2 process
        // planned to use secondary buffer for radix sorting
        auto bounder = accel;
        cmdFillBuffer<0xFFFFFFFFu>(*cmdBuf, *bounder->_mortonCodesBuffer);
        cmdFillBuffer<0u>(*cmdBuf, *bounder->_countersBuffer); // reset counters
        cmdFillBuffer<0u>(*cmdBuf, *bounder->_fitStatusBuffer);
        updateCommandBarrier(*cmdBuf);

        const auto workGroupSize = 16u;
        std::vector<VkDescriptorSet> _sets = { bounder->_buildDescriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        cmdDispatch(*cmdBuf, acclb->_boundingPipeline, 256); // calculate general box of BVH
        cmdDispatch(*cmdBuf, acclb->_shorthandPipeline); // calculate in device boundary results
        cmdDispatch(*cmdBuf, acclb->_leafPipeline, INTENSIVITY); // calculate node boxes and morton codes
        radixSort(cmdBuf, bounder->_sortDescriptorSet, accel->_bvhBlockData.leafCount);
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        cmdDispatch(*cmdBuf, acclb->_buildPipelineFirst, 1); // first few elements
        cmdDispatch(*cmdBuf, acclb->_buildPipeline, workGroupSize); // parallelize by another threads
        cmdDispatch(*cmdBuf, acclb->_leafLinkPipeline, INTENSIVITY); // link leafs
        cmdDispatch(*cmdBuf, acclb->_fitPipeline, workGroupSize);

        return result;
    };

};
