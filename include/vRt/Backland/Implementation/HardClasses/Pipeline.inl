#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;

    // ray tracing pipeline
    // planned to add support of entry points
    VtResult createRayTracingPipeline(std::shared_ptr<Device> _vtDevice,  VtRayTracingPipelineCreateInfo info, std::shared_ptr<Pipeline>& vtPipeline) {
        VtResult result = VK_SUCCESS;

        auto vkDevice = _vtDevice->_device;
        auto vkPipelineCache = _vtDevice->_pipelineCache;

        //auto vtPipeline = (_vtPipeline = std::make_shared<Pipeline>());
        vtPipeline = std::make_shared<Pipeline>();
        vtPipeline->_device = _vtDevice;
        vtPipeline->_tiling = info.tiling; // fixed 15.09.2018
        vtPipeline->_pipelineLayout = std::shared_ptr<PipelineLayout>(info.pipelineLayout);
        const auto vendorName = _vtDevice->_vendorName;

        // generation shaders
        if (info.pGenerationModule) {
            if (info.pGenerationModule[0].module) {
                vtPipeline->_generationPipeline.push_back(createCompute(vkDevice, info.pGenerationModule[0], *vtPipeline->_pipelineLayout, vkPipelineCache));
            }
        }

        // missing shaders
        if (info.pMissModules) {
            for (uint32_t i = 0; i < std::min(1u, info.missModuleCount); i++) {
                if (info.pMissModules[i].module) vtPipeline->_missHitPipeline.push_back(createCompute(vkDevice, info.pMissModules[i], *vtPipeline->_pipelineLayout, vkPipelineCache));
            }
        }

        // hit shaders
        if (info.pClosestModules) {
            for (uint32_t i = 0; i < std::min(4u, info.closestModuleCount); i++) {
                if (info.pClosestModules[i].module) vtPipeline->_closestHitPipeline.push_back(createCompute(vkDevice, info.pClosestModules[i], *vtPipeline->_pipelineLayout, vkPipelineCache));
            }
        }

        // ray groups shaders
        if (info.pGroupModules) {
            for (uint32_t i = 0; i < std::min(4u, info.groupModuleCount); i++) {
                if (info.pGroupModules[i].module) vtPipeline->_groupPipeline.push_back(createCompute(vkDevice, info.pGroupModules[i], *vtPipeline->_pipelineLayout, vkPipelineCache));
            }
        }

        return result;
    };


    RayTracingSet::~RayTracingSet() {
        if (_descriptorSet) vk::Device(VkDevice(*_device)).freeDescriptorSets(_device->_descriptorPool, { vk::DescriptorSet(_descriptorSet) });
        _descriptorSet = {};
    };

    // ray tracing set of state
    VtResult createRayTracingSet(std::shared_ptr<Device> _vtDevice,  VtRayTracingSetCreateInfo info, std::shared_ptr<RayTracingSet>& _vtRTSet) {
        VtResult result = VK_SUCCESS;

        auto vkDevice = _vtDevice->_device;
        auto vtRTSet = (_vtRTSet = std::make_shared<RayTracingSet>());
        vtRTSet->_device = _vtDevice;

        { // planned variable size
            const auto rayCount = info.maxRays, hitCount = info.maxHits ? info.maxHits : rayCount;
            vtRTSet->_cuniform.maxRayCount = rayCount, vtRTSet->_cuniform.maxHitCount = hitCount;

            std::shared_ptr<BufferManager> bManager = {}; createBufferManager(_vtDevice, bManager);

            VtDeviceBufferCreateInfo bfic = {};
            bfic.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);
            
            VtBufferRegionCreateInfo bfi = {};

            
            { // allocate buffer regions
                bfi.bufferSize = rayCount * (8ull * sizeof(uint32_t));
                bfi.format = VK_FORMAT_UNDEFINED;
                createBufferRegion(bManager, bfi, vtRTSet->_rayBuffer);


                bfi.bufferSize = rayCount * MAX_RAY_GROUPS * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_groupIndicesBuffer);
                createBufferRegion(bManager, bfi, vtRTSet->_groupIndicesBufferRead);


                bfi.format = VK_FORMAT_UNDEFINED;
                bfi.bufferSize = hitCount * (16ull * sizeof(uint32_t));
                createBufferRegion(bManager, bfi, vtRTSet->_hitBuffer);


                bfi.bufferSize = hitCount * 5ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_closestHitIndiceBuffer);


                bfi.bufferSize = hitCount * 1ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_missedHitIndiceBuffer);


                bfi.bufferSize = 16ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_countersBuffer);


                bfi.bufferSize = 64ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_groupCountersBuffer);


                bfi.bufferSize = 64ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_groupCountersBufferRead);


                bfi.bufferSize = hitCount * 16ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createBufferRegion(bManager, bfi, vtRTSet->_hitPayloadBuffer);


                bfi.bufferSize = rayCount * 4ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_rayLinkPayload);


                // ray traversing local cache extension
                constexpr auto LOCAL_SIZE = 1024ull, STACK_SIZE = 8ull, PAGE_COUNT = 16ull, STATE_COUNT = 1ull;
                bfi.bufferSize = (RV_INTENSIVITY * STACK_SIZE * LOCAL_SIZE * (PAGE_COUNT+1ull) * STATE_COUNT * sizeof(uint32_t))<<0ull;
                bfi.format = VK_FORMAT_R32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_traverseCache);


                bfi.bufferSize = sizeof(VtStageUniform);
                bfi.format = VK_FORMAT_UNDEFINED;
                createBufferRegion(bManager, bfi, vtRTSet->_constBuffer);

                // task lists 
                //bfi.bufferSize = (hitCount * sizeof(uint64_t)) << 1ull; // max 2 tasks supported now 
                bfi.bufferSize = sizeof(uint64_t);
                bfi.format = VK_FORMAT_R32G32_UINT;
                createBufferRegion(bManager, bfi, vtRTSet->_taskBuffer);


                bfi.bufferSize = hitCount * ATTRIB_EXTENT * (4ull * sizeof(uint32_t));
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createBufferRegion(bManager, bfi, vtRTSet->_attribBuffer);


                // build final shared buffer for this class
                createSharedBuffer(bManager, bfic, vtRTSet->_sharedBuffer);
            };

            {
                std::vector<vk::DescriptorSetLayout> dsLayouts = { vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]), };
                const auto&& dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtRTSet->_descriptorSet = std::move(dsc[0]);

                // write descriptors
                const auto writeTmpl = vk::WriteDescriptorSet(vtRTSet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(10).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&vtRTSet->_rayLinkPayload->_bufferView()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(11).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&vtRTSet->_attribBuffer->_bufferView()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(9).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_traverseCache->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_rayBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_hitBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_closestHitIndiceBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_missedHitIndiceBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_hitPayloadBuffer->_descriptorInfo()),
                    
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_constBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_countersBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(8).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_taskBuffer->_descriptorInfo()),

                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(5 ).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_groupIndicesBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(13).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_groupIndicesBufferRead->_descriptorInfo()),
                    
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(12).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_groupCountersBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(14).setPBufferInfo((vk::DescriptorBufferInfo*)&vtRTSet->_groupCountersBufferRead->_descriptorInfo()),
                };
                vk::Device(vkDevice).updateDescriptorSets(writes, {});
            };
        };

        return result;
    };
};
