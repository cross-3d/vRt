#pragma once

#include "../../vRt_subimpl.inl"
#include "RadixSort.inl"

namespace _vt {
    using namespace vt;


    VtResult bindVertexInputs(std::shared_ptr<CommandBuffer>& cmdBuf, const std::vector<std::shared_ptr<VertexInputSet>>& sets) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexInputs.resize(0);
        for (auto& s : sets) { // update buffers by pushing constants
            vkCmdUpdateBuffer(*cmdBuf, *s->_uniformBlockBuffer, 0, sizeof(s->_uniformBlock), &s->_uniformBlock);
            fromHostCommandBarrier(*cmdBuf);
            cmdBuf->_vertexInputs.push_back(s);
        }
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
        imageBarrier(*cmdBuf, vasSet->_attributeTexelBuffer);
        return result;
    }


    VtResult buildVertexSet(std::shared_ptr<CommandBuffer>& cmdBuf) {
        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        auto vertb = device->_vertexAssembler;
        auto vertx = cmdBuf->_vertexSet.lock();
        cmdFillBuffer<0u>(*cmdBuf, *vertx->_countersBuffer);
        vertx->_calculatedPrimitiveCount = 0;
        for (auto& iV_ : cmdBuf->_vertexInputs) {
			auto iV = iV_.lock();
            std::vector<VkDescriptorSet> _sets = { vertx->_descriptorSet, iV->_descriptorSet };
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, vertb->_vertexAssemblyPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
            cmdDispatch(*cmdBuf, vertb->_vertexAssemblyPipeline, 4096);
            cmdCopyBuffer(*cmdBuf, vertx->_countersBuffer, vertx->_countersBuffer, { vk::BufferCopy(strided<uint32_t>(0), strided<uint32_t>(1), strided<uint32_t>(1)) });
            vertx->_calculatedPrimitiveCount += iV->_uniformBlock.primitiveCount;
        }
        return result;
    }


    VtResult buildAccelerator(std::shared_ptr<CommandBuffer>& cmdBuf) {
        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        auto acclb = device->_acceleratorBuilder;
        auto accel = cmdBuf->_acceleratorSet.lock();
		auto vertx = cmdBuf->_vertexSet.lock();

        // copy vertex assembly counter values
        cmdCopyBuffer(*cmdBuf, vertx->_countersBuffer, accel->_bvhBlockUniform, { vk::BufferCopy(strided<uint32_t>(0), strided<uint32_t>(64+0), strided<uint32_t>(1)) });
        cmdCopyBuffer(*cmdBuf, vertx->_countersBuffer, accel->_bvhBlockUniform, { vk::BufferCopy(strided<uint32_t>(0), strided<uint32_t>(64+1), strided<uint32_t>(1)) });

        // building hlBVH2 process
        // planned to use secondary buffer for radix sorting
        cmdFillBuffer<0xFFu>(*cmdBuf, *acclb->_mortonCodesBuffer);
        cmdFillBuffer<0u>(*cmdBuf, *acclb->_countersBuffer); // reset counters
        std::vector<VkDescriptorSet> _sets = { acclb->_buildDescriptorSet, accel->_descriptorSet, vertx->_descriptorSet };
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        cmdDispatch(*cmdBuf, acclb->_boundingPipeline, 128); // calculate general box of BVH
        cmdDispatch(*cmdBuf, acclb->_shorthandPipeline); // calculate in device boundary results
        cmdDispatch(*cmdBuf, acclb->_leafPipeline, 4096); // calculate node boxes and morton codes
        radixSort(cmdBuf, acclb->_sortDescriptorSet, vertx->_calculatedPrimitiveCount);
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
        cmdDispatch(*cmdBuf, acclb->_buildPipeline, 1); // just build hlBVH2
        cmdDispatch(*cmdBuf, acclb->_leafLinkPipeline, 4096); // link leafs
        cmdDispatch(*cmdBuf, acclb->_fitPipeline, 4096); // fit BVH nodes

        return result;
    }

};
