#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // ray tracing pipeline
    // planned to add support of entry points
    inline VtResult createRayTracingPipeline(std::shared_ptr<Device> _vtDevice, const VtRayTracingPipelineCreateInfo& info, std::shared_ptr<Pipeline>& _vtPipeline) {
        VtResult result = VK_SUCCESS;

        auto& vtPipeline = (_vtPipeline = std::make_shared<Pipeline>());
        vtPipeline->_device = _vtDevice;
        vtPipeline->_pipelineLayout = info.pipelineLayout;

        //vtPipeline->_closestHitPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + info.closestShader, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));
        //vtPipeline->_missHitPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + info.missShader, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));
        //vtPipeline->_generationPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + info.generationShader, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));
        //vtPipeline->_resolvePipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + info.resolveShader, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));

        vtPipeline->_closestHitPipeline = createCompute(VkDevice(*_vtDevice), info.closestModule, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtPipeline->_missHitPipeline = createCompute(VkDevice(*_vtDevice), info.missModule, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtPipeline->_generationPipeline = createCompute(VkDevice(*_vtDevice), info.generationModule, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));
        vtPipeline->_resolvePipeline = createCompute(VkDevice(*_vtDevice), info.resolveModule, *vtPipeline->_pipelineLayout, VkPipelineCache(*_vtDevice));

        return result;
    }

    // ray tracing set of state
    inline VtResult createRayTracingSet(std::shared_ptr<Device> _vtDevice, const VtRayTracingSetCreateInfo& info, std::shared_ptr<RayTracingSet>& _vtRTSet) {
        VtResult result = VK_SUCCESS;

        auto& vtRTSet = (_vtRTSet = std::make_shared<RayTracingSet>());
        vtRTSet->_device = _vtDevice;

        // planned variable size

        {
            const auto& rayCount = info.maxRays;

            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);


                bfi.bufferSize = rayCount * 32;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_rayBuffer);


                bfi.bufferSize = rayCount * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_rayIndiceBuffer);


                bfi.bufferSize = rayCount * 128;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_hitBuffer);


                bfi.bufferSize = rayCount * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_closestHitIndiceBuffer);


                bfi.bufferSize = rayCount * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_missedHitIndiceBuffer);


                bfi.bufferSize = 8 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_countersBuffer);


                bfi.bufferSize = rayCount * 128;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_hitPayloadBuffer);


                bfi.bufferSize = 1024 * 256 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32G32B32A32_SINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_traverseCache);


                bfi.bufferSize = 8 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_constBuffer);


                bfi.bufferSize = tiled(rayCount, 4096ull) * 4096ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtRTSet->_blockBuffer);
            };

            {
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
                };
                auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtRTSet->_descriptorSet = dsc[0];


                vk::Sampler attributeSampler = vk::Device(*_vtDevice).createSampler(vk::SamplerCreateInfo()
                    .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                    .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                    .setMagFilter(vk::Filter::eNearest)
                    .setMinFilter(vk::Filter::eNearest)
                );
                auto _write_tmpl = vk::WriteDescriptorSet(vtRTSet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(7).setDescriptorType(vk::DescriptorType::eStorageBufferDynamic).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_countersBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(9).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtRTSet->_traverseCache->_bufferView)),

                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_rayBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_hitBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_closestHitIndiceBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_missedHitIndiceBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_hitPayloadBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_rayIndiceBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_constBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(8).setPBufferInfo(&vk::DescriptorBufferInfo(vtRTSet->_blockBuffer->_descriptorInfo())),
                };
                vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
            };
        }


        return result;
    }

};
