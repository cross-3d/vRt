#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"
#include "RadixSort.inl" // TODO: dedicated implementation

namespace _vt {
    using namespace vrt;


    // 
    VtResult bindDescriptorSetsPerVertexInput(const std::shared_ptr<CommandBuffer>& cmdBuf, VtPipelineBindPoint pipelineBindPoint, VtPipelineLayout layout, uint32_t vertexInputID = 0, uint32_t firstSet = 0, const std::vector<VkDescriptorSet>& descriptorSets = {}, const std::vector<VkDescriptorSet>& dynamicOffsets = {}) {
        VtResult result = VK_SUCCESS;
        if (pipelineBindPoint == VT_PIPELINE_BIND_POINT_VERTEXASSEMBLY) {
            cmdBuf->_perVertexInputDSC[vertexInputID] = descriptorSets;
        };
        return result;
    };

    VtResult bindVertexInputs(const std::shared_ptr<CommandBuffer>& cmdBuf, const std::vector<std::shared_ptr<VertexInputSet>>& sets) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexInputs = sets;
        return result;
    };

    VtResult bindAccelerator(const std::shared_ptr<CommandBuffer>& cmdBuf, std::shared_ptr<AcceleratorSet> accSet) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_acceleratorSet = accSet;
        return result;
    };

    // bind vertex assembly (also, do imageBarrier)
    VtResult bindVertexAssembly(const std::shared_ptr<CommandBuffer>& cmdBuf, std::shared_ptr<VertexAssemblySet> vasSet) {
        VtResult result = VK_SUCCESS;
        cmdBuf->_vertexSet = vasSet;
        return result;
    };

    VtResult cmdVertexAssemblyBarrier(VkCommandBuffer cmdBuf, const std::shared_ptr<VertexAssemblySet>& vasSet) {
        VtResult result = VK_SUCCESS;
        if (vasSet->_attributeTexelBuffer) imageBarrier(cmdBuf, vasSet->_attributeTexelBuffer);
        return result;
    };

    
    // update region of vertex set by bound input set
    VtResult updateVertexSet(const std::shared_ptr<CommandBuffer>& cmdBuf, std::function<void(VkCommandBuffer, int, VtUniformBlock&)> cb = {}) {
        VtResult result = VK_SUCCESS;

        // useless to building
        if (cmdBuf->_vertexInputs.size() <= 0) return result;

        auto device = cmdBuf->_parent();
        auto vertx = cmdBuf->_vertexSet.lock();
        auto vasmp = vertx && vertx->_vertexAssembly ? vertx->_vertexAssembly : device->_nativeVertexAssembler[0];
        vertx->_vertexInputs = cmdBuf->_vertexInputs;

        // update constants
        uint32_t _bndc = 0, calculatedPrimitiveCount = 0;
        std::vector<VtUniformBlock> uniformData = {};
        for (auto iV : cmdBuf->_vertexInputs) {
            const uint32_t _bnd = _bndc++;

            //iV->_uniformBlock.inputCount = cmdBuf->_vertexInputs.size();
            iV->_uniformBlock.primitiveOffset = calculatedPrimitiveCount;
            if (cb) { cb(*cmdBuf, int(_bnd), iV->_uniformBlock); };
            uniformData.push_back(iV->_uniformBlock);

            calculatedPrimitiveCount += iV->_uniformBlock.primitiveCount;
        }; _bndc = 0;
        vertx->_calculatedPrimitiveCount = calculatedPrimitiveCount;

        // copy to staging buffers
        memcpy(vertx->_bufferTraffic[0]->_uniformVIMapped->_hostMapped(), uniformData.data(), uniformData.size() * sizeof(VtUniformBlock));
        return result;
    };

    // 
    VtResult buildVertexSet(const std::shared_ptr<CommandBuffer>& cmdBuf, bool useInstance = true, std::function<void(VkCommandBuffer, int, VtUniformBlock&)> cb = {}) {
        VtResult result = VK_SUCCESS;

        // useless to building
        if (cmdBuf->_vertexInputs.size() <= 0) return result;

        auto device = cmdBuf->_parent();
        auto vertx = cmdBuf->_vertexSet.lock();
        auto vasmp = vertx && vertx->_vertexAssembly ? vertx->_vertexAssembly : device->_nativeVertexAssembler[0];
        vertx->_vertexInputs = cmdBuf->_vertexInputs;

        // update vertex buffer 
        result = updateVertexSet(cmdBuf, cb);

        // image barrier
        if (vertx->_attributeTexelBuffer) imageBarrier(*cmdBuf, vertx->_attributeTexelBuffer);
        updateCommandBarrier(*cmdBuf);

        // copy command to 
        cmdCopyBufferFromHost(*cmdBuf, vertx->_bufferTraffic[0]->_uniformVIMapped, vertx->_bufferTraffic[0]->_uniformVIBuffer, { vk::BufferCopy(0u, 0u, cmdBuf->_vertexInputs.size() * sizeof(VtUniformBlock)) });

        // 
        if (vertx->_descriptorSetGenerator) {
            vertx->_descriptorSetGenerator();
        };

        
        //if (useInstance) {
            //const uint32_t _bnd = 0, _szi = cmdBuf->_vertexInputs.size();
            std::vector<uint32_t> bdata{ 0, uint32_t(cmdBuf->_vertexInputs.size()) };
            const uint32_t& _bnd = bdata[0], _szi = bdata[1];

            // native descriptor sets
            auto iV = cmdBuf->_vertexInputs[_bnd];
            if (iV->_descriptorSetGenerator) iV->_descriptorSetGenerator();
            std::vector<VkDescriptorSet> _sets = { device->_emptyDS, iV->_descriptorSet, vertx->_descriptorSet };

            // user defined descriptor sets
            auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
            for (auto s : _bsets) { _sets.push_back(s); };

            // 
            const auto pLayout = (iV->_attributeAssembly ? iV->_attributeAssembly : vasmp)->_pipelineLayout;
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *pLayout, 0, _sets.size(), _sets.data(), 0, nullptr); // bind descriptor sets

            vkCmdPushConstants(*cmdBuf, *pLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, bdata.size()*sizeof(uint32_t), bdata.data());
            cmdDispatch(*cmdBuf, vasmp->_inputPipeline, VX_INTENSIVITY, 1, 1, false);
            if (iV->_attributeAssembly) cmdDispatch(*cmdBuf, iV->_attributeAssembly->_inputPipeline, VX_INTENSIVITY, 1, 1, false);
        /*} else {
            uint32_t _bndc = 0u;
            for (auto iV : cmdBuf->_vertexInputs) {
                const uint32_t _bnd = _bndc++;

                // native descriptor sets
                if (iV->_descriptorSetGenerator) iV->_descriptorSetGenerator();
                std::vector<VkDescriptorSet> _sets = { device->_emptyDS, iV->_descriptorSet, vertx->_descriptorSet };

                // user defined descriptor sets
                auto _bsets = cmdBuf->_perVertexInputDSC.find(_bnd) != cmdBuf->_perVertexInputDSC.end() ? cmdBuf->_perVertexInputDSC[_bnd] : cmdBuf->_boundVIDescriptorSets;
                for (auto s : _bsets) { _sets.push_back(s); };

                // execute vertex assembly
                const auto pLayout = (iV->_attributeAssembly ? iV->_attributeAssembly : vasmp)->_pipelineLayout;
                vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, *pLayout, 0, _sets.size(), _sets.data(), 0, nullptr); // bind descriptor sets
                vkCmdPushConstants(*cmdBuf, *pLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &_bnd);
                cmdDispatch(*cmdBuf, vasmp->_inputPipeline, VX_INTENSIVITY, 1, 1, false);
                if (iV->_attributeAssembly) cmdDispatch(*cmdBuf, iV->_attributeAssembly->_inputPipeline, VX_INTENSIVITY, 1, 1, false);
            }
        };*/
        commandBarrier(*cmdBuf);

        return result;
    };




    // building accelerator structure
    // TODO: enable AABB shaders for real support of multi-leveling (i.e. top level)
    VtResult buildAccelerator(const std::shared_ptr<CommandBuffer>& cmdBuf, const VtAcceleratorBuildInfo& buildInfo = {}) {
        VtResult result = VK_SUCCESS;
        auto device = cmdBuf->_parent();
        //auto rtset = cmdBuf->_rayTracingSet.lock();
        auto acclb = device->_acceleratorBuilder[0];
        auto accel = cmdBuf->_acceleratorSet.lock();
        auto vertx = cmdBuf->_vertexSet.lock();
        if (vertx) accel->_vertexAssemblySet = vertx; // bind vertex assembly with accelerator structure (planned to deprecate)


        // if has advanced accelerator
        if (device->_hExtensionAccelerator.size() > 0 && device->_hExtensionAccelerator[0] && device->_enabledAdvancedAcceleration) {
            result = device->_hExtensionAccelerator[0]->_BuildAccelerator(cmdBuf, accel, buildInfo);
        }
        else {

            // if geometry level, limit by vertex assembly set 
            VkDeviceSize vsize = vertx && accel->_level == VT_ACCELERATOR_SET_LEVEL_GEOMETRY ? VkDeviceSize(vertx->_calculatedPrimitiveCount) : VK_WHOLE_SIZE;

            //VtBuildConst _buildConstData = {};
            VtBuildConst& _buildConstData = acclb->_buildConstData;
            _buildConstData.primitiveOffset = buildInfo.elementOffset + accel->_elementsOffset; // 
            _buildConstData.primitiveCount = std::min((accel->_elementsCount != -1 && accel->_elementsCount >= 0) ? VkDeviceSize(accel->_elementsCount) : VkDeviceSize(vsize), std::min(buildInfo.elementSize, accel->_capacity));

            // create BVH instance meta (linking with geometry) 
            //VtBvhBlock _bvhBlockData = {};
            VtBvhBlock& _bvhBlockData = accel->_bvhBlockData;
            _bvhBlockData.elementsOffset = _buildConstData.primitiveOffset;
            _bvhBlockData.elementsCount = _buildConstData.primitiveCount;
            _bvhBlockData.entryID = accel->_entryID;

            // planned to merge instance buffer of linked set
            _bvhBlockData.transform = VtMat3x4{ accel->_coverMatrice.m[0], accel->_coverMatrice.m[1], accel->_coverMatrice.m[2] };
            //_bvhBlockData.transformInv = IdentifyMat4; 

            // updating meta buffers
            cmdUpdateBuffer(*cmdBuf, accel->_bvhHeadingBuffer, 0, sizeof(_bvhBlockData), &_bvhBlockData);
            cmdUpdateBuffer(*cmdBuf, acclb->_constBuffer, 0, sizeof(_buildConstData), &_buildConstData);


            // building hlBVH2 process
            // planned to use secondary buffer for radix sorting
            //auto bounder = accel;

            // incorrect sorting defence ( can't help after sorting )
            //cmdFillBuffer<0xFFFFFFFFu>(*cmdBuf, acclb->_mortonCodesBuffer);

            // only for debug ( for better visibility )
            //cmdFillBuffer<0u>(*cmdBuf, acclb->_mortonIndicesBuffer);
            //cmdFillBuffer<0u>(*cmdBuf, accel->_bvhBoxBuffer);

            cmdFillBuffer<0u>(*cmdBuf, acclb->_countersBuffer); // reset counters
            cmdFillBuffer<0u>(*cmdBuf, acclb->_fitStatusBuffer);
            updateCommandBarrier(*cmdBuf);

            const auto workGroupSize = 16u;
            if (accel->_descriptorSetGenerator) accel->_descriptorSetGenerator();
            std::vector<VkDescriptorSet> _sets = { acclb->_buildDescriptorSet, accel->_descriptorSet };

            if (vertx) {
                if (vertx->_descriptorSetGenerator) { vertx->_descriptorSetGenerator(); };
                if (vertx->_descriptorSet) { _sets.push_back(vertx->_descriptorSet); };
            };
            
            vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
            cmdDispatch(*cmdBuf, acclb->_boxCalcPipeline[accel->_level], INTENSIVITY); // calculate general box of BVH
            cmdDispatch(*cmdBuf, acclb->_boundingPipeline, 256); // calculate general box of BVH
            cmdDispatch(*cmdBuf, acclb->_shorthandPipeline); // calculate in device boundary results
            cmdDispatch(*cmdBuf, acclb->_leafPipeline[accel->_level], INTENSIVITY); // calculate node boxes and morton codes
            if (_bvhBlockData.elementsCount > 2) { // don't use radix sort when only 1-2 elements 
                radixSort(cmdBuf, acclb->_sortDescriptorSet, _bvhBlockData.elementsCount);
                vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, acclb->_buildPipelineLayout, 0, _sets.size(), _sets.data(), 0, nullptr);
            };
            cmdDispatch(*cmdBuf, acclb->_buildPipelineFirst, 1); // first few elements
            if (_bvhBlockData.elementsCount > workGroupSize) { // useless step for too fews 
                cmdDispatch(*cmdBuf, acclb->_buildPipeline, workGroupSize); // parallelize by another threads
            };
            cmdDispatch(*cmdBuf, acclb->_leafLinkPipeline, INTENSIVITY); // link leafs
            cmdDispatch(*cmdBuf, acclb->_fitPipeline, INTENSIVITY);
        };
        //commandBarrier(*cmdBuf);

        return result;
    };
};
