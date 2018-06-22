#pragma once

#include "../../vRt_subimpl.inl"
#include "RadixSort.inl"

namespace _vt {
    using namespace vt;



    VtResult bindDescriptorSetsPerVertexInput(std::shared_ptr<CommandBuffer>& cmdBuf, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t vertexInputID = 0, uint32_t firstSet = 0, const std::vector<VkDescriptorSet>& descriptorSets = {}, const std::vector<VkDescriptorSet>& dynamicOffsets = {}) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_VERTEXASSEMBLY) {
            cmdBuf->_perVertexInputDSC[vertexInputID] = descriptorSets;
        };
        return result;
    }

    VtResult bindVertexInputs(std::shared_ptr<CommandBuffer>& cmdBuf, const std::vector<std::shared_ptr<VertexInputSet>>& sets) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexInputs.resize(0);
        for (auto& s : sets) { // update buffers by pushing constants
            //s->_uniformBlock.inputID = cmdBuf->_vertexInputs.size();
            //vkCmdUpdateBuffer(*cmdBuf, *s->_uniformBlockBuffer, 0, sizeof(s->_uniformBlock), &s->_uniformBlock);
            //fromHostCommandBarrier(*cmdBuf);
            cmdBuf->_vertexInputs.push_back(s);
        }; 
        return result;
    }


    VtResult bindAccelerator(std::shared_ptr<CommandBuffer>& cmdBuf, std::shared_ptr<AcceleratorSet>& accSet) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_acceleratorSet = accSet;
        return result;
    }

    // bind vertex assembly (also, do imageBarrier)
    VtResult bindVertexAssembly(std::shared_ptr<CommandBuffer>& cmdBuf, std::shared_ptr<VertexAssemblySet>& vasSet) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexSet = vasSet;
        //imageBarrier(*cmdBuf, vasSet->_attributeTexelBuffer);
        return result;
    }

    VtResult cmdVertexAssemblyBarrier(VkCommandBuffer cmdBuf, std::shared_ptr<VertexAssemblySet>& vasSet) {
        VtResult result = VK_SUCCESS;
        imageBarrier(cmdBuf, vasSet->_attributeTexelBuffer);
        return result;
    }


    VtResult buildVertexSet(std::shared_ptr<CommandBuffer>& cmdBuf) {
        VtResult result = VK_SUCCESS;

        // useless to building
        if (cmdBuf->_vertexInputs.size() <= 0) return result;
        

        auto device = cmdBuf->_parent();
        auto vertbd = device->_vertexAssembler;
        auto vertx = cmdBuf->_vertexSet.lock();

        imageBarrier(*cmdBuf, vertx->_attributeTexelBuffer);
        cmdFillBuffer<0u>(*cmdBuf, *vertx->_countersBuffer);
        vertx->_calculatedPrimitiveCount = 0;

        uint32_t _bndc = 0, calculatedPrimitiveCount = 0;
        for (auto& iV_ : cmdBuf->_vertexInputs) {
            uint32_t _bnd = _bndc++;
            auto iV = iV_.lock();

            
            
            auto vertb = iV->_vertexAssembly ? iV->_vertexAssembly : vertbd;
            std::vector<VkDescriptorSet> _sets = { vertx->_descriptorSet, iV->_descriptorSet };

            auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
            for (auto &s : _bsets) { _sets.push_back(s); }

            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *vertb->_pipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);


            iV->_uniformBlock.inputID = _bnd;
            vkCmdUpdateBuffer(*cmdBuf, *iV->_uniformBlockBuffer, 0, sizeof(iV->_uniformBlock), &iV->_uniformBlock);
            commandBarrier(*cmdBuf); //fromHostCommandBarrier(*cmdBuf);
            //vkCmdPushConstants(*cmdBuf, *vertb->_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(iV->_uniformBlock), &iV->_uniformBlock);


            cmdDispatch(*cmdBuf, vertb->_vertexAssemblyPipeline, INTENSIVITY);
            cmdCopyBuffer(*cmdBuf, vertx->_countersBuffer, vertx->_countersBuffer, { vk::BufferCopy(strided<uint32_t>(0), strided<uint32_t>(1), strided<uint32_t>(1)) });
            calculatedPrimitiveCount += iV->_uniformBlock.primitiveCount;
        }

        vertx->_calculatedPrimitiveCount = calculatedPrimitiveCount;

        return result;
    }


    VtResult buildAccelerator(std::shared_ptr<CommandBuffer>& cmdBuf) {
        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        auto acclb = device->_acceleratorBuilder;
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();

        // build vertex if possible
        if (vertx && cmdBuf->_vertexInputs.size() > 0) {
            buildVertexSet(cmdBuf);
        }

        // copy vertex assembly counter values
        cmdCopyBuffer(*cmdBuf, vertx->_countersBuffer, accel->_bvhBlockUniform, { vk::BufferCopy(strided<uint32_t>(0), strided<uint32_t>(64 + 0), strided<uint32_t>(1)) });
        cmdCopyBuffer(*cmdBuf, vertx->_countersBuffer, accel->_bvhBlockUniform, { vk::BufferCopy(strided<uint32_t>(0), strided<uint32_t>(64 + 1), strided<uint32_t>(1)) });

        // building hlBVH2 process
        // planned to use secondary buffer for radix sorting
        cmdFillBuffer<0xFFFFFFFFu>(*cmdBuf, *acclb->_mortonCodesBuffer);
        cmdFillBuffer<0u>(*cmdBuf, *acclb->_countersBuffer); // reset counters
        cmdFillBuffer<0u>(*cmdBuf, *acclb->_fitStatusBuffer);
        std::vector<VkDescriptorSet> _sets = { acclb->_buildDescriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        cmdDispatch(*cmdBuf, acclb->_boundingPipeline, 128); // calculate general box of BVH
        cmdDispatch(*cmdBuf, acclb->_shorthandPipeline); // calculate in device boundary results
        cmdDispatch(*cmdBuf, acclb->_leafPipeline, INTENSIVITY); // calculate node boxes and morton codes
        radixSort(cmdBuf, acclb->_sortDescriptorSet, vertx->_calculatedPrimitiveCount);
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        cmdDispatch(*cmdBuf, acclb->_buildPipeline, 1); // just build hlBVH2
        cmdDispatch(*cmdBuf, acclb->_leafLinkPipeline, INTENSIVITY); // link leafs
        cmdDispatch(*cmdBuf, acclb->_fitPipeline, 1);
        //cmdDispatch(*cmdBuf, acclb->_fitPipeline, INTENSIVITY); // fit BVH nodes

        return result;
    }

};
