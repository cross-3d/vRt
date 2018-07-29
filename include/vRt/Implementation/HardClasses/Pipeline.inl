#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // ray tracing pipeline
    // planned to add support of entry points
    static inline VtResult createRayTracingPipeline(std::shared_ptr<Device> _vtDevice, const VtRayTracingPipelineCreateInfo& info, std::shared_ptr<Pipeline>& _vtPipeline) {
        VtResult result = VK_SUCCESS;

        auto vkDevice = _vtDevice->_device;
        auto vkPipelineCache = _vtDevice->_pipelineCache;

        auto& vtPipeline = (_vtPipeline = std::make_shared<Pipeline>());
        vtPipeline->_device = _vtDevice;
        vtPipeline->_pipelineLayout = info.pipelineLayout._vtPipelineLayout;
        const auto& vendorName = _vtDevice->_vendorName;

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
                if (info.pGroupModules[i].module) vtPipeline->_groupPipelines.push_back(createCompute(vkDevice, info.pGroupModules[i], *vtPipeline->_pipelineLayout, vkPipelineCache));
            }
        }

        return result;
    }

    // ray tracing set of state
    static inline VtResult createRayTracingSet(std::shared_ptr<Device> _vtDevice, const VtRayTracingSetCreateInfo& info, std::shared_ptr<RayTracingSet>& _vtRTSet) {
        VtResult result = VK_SUCCESS;

        auto vkDevice = _vtDevice->_device;
        auto& vtRTSet = (_vtRTSet = std::make_shared<RayTracingSet>());
        vtRTSet->_device = _vtDevice;

        { // planned variable size
            const auto& rayCount = info.maxRays;
            vtRTSet->_cuniform.maxRayCount = rayCount;
            
            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);


                bfi.bufferSize = rayCount * 32ull;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_rayBuffer);


                bfi.bufferSize = rayCount * 4ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_groupIndicesBuffer);


                bfi.bufferSize = rayCount * 4ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_groupIndicesBufferRead);


                bfi.bufferSize = rayCount * 16ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_hitBuffer);


                bfi.bufferSize = rayCount * 5ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_closestHitIndiceBuffer);


                bfi.bufferSize = rayCount * 5ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_missedHitIndiceBuffer);


                bfi.bufferSize = 16ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_countersBuffer);


                bfi.bufferSize = 16ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_groupCountersBuffer);


                bfi.bufferSize = 16ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_groupCountersBufferRead);


                bfi.bufferSize = rayCount * 64ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_hitPayloadBuffer);


                bfi.bufferSize = rayCount * 2ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32G32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_rayLinkPayload);


                bfi.bufferSize = 4096ull * 4096ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_SINT;
                //bfi.format = VK_FORMAT_R32G32B32A32_SINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_traverseCache);


                bfi.bufferSize = sizeof(VtStageUniform);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_constBuffer);

                // at now unused
                bfi.bufferSize = sizeof(uint32_t);//tiled(rayCount, 4096ull) * 4096ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_blockBuffer);


                bfi.bufferSize = rayCount * 4ull * ATTRIB_EXTENT * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_attribBuffer);
            };

            {
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
                };
                auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtRTSet->_descriptorSet = dsc[0];

                vk::Sampler attributeSampler = vk::Device(vkDevice).createSampler(vk::SamplerCreateInfo()
                    .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                    .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                    .setMagFilter(vk::Filter::eNearest)
                    .setMinFilter(vk::Filter::eNearest)
                );
                auto _write_tmpl = vk::WriteDescriptorSet(vtRTSet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    //vk::WriteDescriptorSet(_write_tmpl).setDstBinding(9).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtRTSet->_traverseCache->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(10).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtRTSet->_rayLinkPayload->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(11).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtRTSet->_attribBuffer->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(9).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_traverseCache->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_rayBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_hitBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_closestHitIndiceBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_missedHitIndiceBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_hitPayloadBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_groupIndicesBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_constBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(7).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_countersBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(8).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_blockBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(12).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_groupCountersBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(13).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_groupIndicesBufferRead->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(14).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_groupCountersBufferRead->_descriptorInfo())),
                };
                vk::Device(vkDevice).updateDescriptorSets(writes, {});
            };
        }

        return result;
    }
};
