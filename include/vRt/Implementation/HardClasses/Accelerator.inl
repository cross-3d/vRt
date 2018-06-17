#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    inline VtResult createAcceleratorHLBVH2(std::shared_ptr<Device> _vtDevice, const VtArtificalDeviceExtension& info, std::shared_ptr<AcceleratorHLBVH2>& _vtAccelerator) {
        VtResult result = VK_SUCCESS;
        auto& vtAccelerator = (_vtAccelerator = std::make_shared<AcceleratorHLBVH2>());
        vtAccelerator->_device = _vtDevice;

        // planned import from descriptor
        //constexpr auto maxPrimitives = 1024u * 1024u;
        const auto& maxPrimitives = info.maxPrimitives;

        // build BVH builder program
        {
            {
                std::vector<vk::PushConstantRange> constRanges = {
                    vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
                };
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2work"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"])
                };

                // create pipeline layout
                vtAccelerator->_buildPipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
                auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtAccelerator->_buildDescriptorSet = dsc[0];
            };

            {
                std::vector<vk::PushConstantRange> constRanges = {
                    //vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
                };
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                };

                // create pipeline layout
                vtAccelerator->_traversePipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
            };

            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_leafBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint64_t);
                bfi.format = VK_FORMAT_R32G32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_mortonCodesBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_SINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_mortonIndicesBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16 * 2;
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_onWorkBoxes);

                bfi.bufferSize = maxPrimitives * 2 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_currentNodeIndices);

                bfi.bufferSize = maxPrimitives * 2 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_fitStatusBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_leafNodeIndices);

                bfi.bufferSize = 16 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_R32_UINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_countersBuffer);

                bfi.bufferSize = 2 * 128 * 16 * sizeof(float);
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_generalBoundaryResultBuffer);
            };

            {
                auto _write_tmpl = vk::WriteDescriptorSet(vtAccelerator->_buildDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(8).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_countersBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonCodesBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonIndicesBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_leafBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_onWorkBoxes->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_fitStatusBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_currentNodeIndices->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(7).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_leafNodeIndices->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(9).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_generalBoundaryResultBuffer->_descriptorInfo())),
                };
                vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
            };

            // create pipelines (planned to unify between accelerator instances)
            {
                vtAccelerator->_shorthandPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/shorthand.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_boundingPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/bound-calc.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_buildPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/bvh-build.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_fitPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/bvh-fit.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_leafPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/leaf-gen.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_leafLinkPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/leaf-link.comp.spv", vtAccelerator->_buildPipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_intersectionPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/traverse-bvh.comp.spv", vtAccelerator->_traversePipelineLayout, VkPipelineCache(*_vtDevice));
                vtAccelerator->_interpolatorPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "hlBVH2/interpolator.comp.spv", vtAccelerator->_traversePipelineLayout, VkPipelineCache(*_vtDevice));
            };
        };


        // write radix sort descriptor sets
        {
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["radixSortBind"]),
            };
            auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtAccelerator->_sortDescriptorSet = dsc[0];

            auto _write_tmpl = vk::WriteDescriptorSet(vtAccelerator->_sortDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonCodesBuffer->_descriptorInfo())), //unused
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_mortonIndicesBuffer->_descriptorInfo()))
            };
            vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
        };

        return result;
    };




    inline VtResult createAcceleratorSet(std::shared_ptr<Device> _vtDevice, const VtAcceleratorSetCreateInfo &info, std::shared_ptr<AcceleratorSet>& _vtAccelerator) {
        VtResult result = VK_SUCCESS;
        auto& vtAccelerator = (_vtAccelerator = std::make_shared<AcceleratorSet>());
        vtAccelerator->_device = _vtDevice;

        // planned import from descriptor
        constexpr auto maxPrimitives = 1024u * 1024u;

        // build BVH builder program
        {
            {
                std::vector<vk::PushConstantRange> constRanges = {
                    vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(2))
                };
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"])
                };

                // create descriptor set
                auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtAccelerator->_descriptorSet = dsc[0];
            };

            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);


                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 4 * 2;
                bfi.format = VK_FORMAT_R32G32B32A32_SINT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_bvhMetaBuffer);

                bfi.bufferSize = maxPrimitives * sizeof(uint32_t) * 16 * 2;
                bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_bvhBoxBuffer);

                bfi.bufferSize = sizeof(uint32_t) * 128;
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtAccelerator->_bvhBlockUniform);
            };

            {
                auto _write_tmpl = vk::WriteDescriptorSet(vtAccelerator->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtAccelerator->_bvhMetaBuffer->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&vk::BufferView(vtAccelerator->_bvhMetaBuffer->_bufferView)),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_bvhBoxBuffer->_descriptorInfo())),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setPBufferInfo(&vk::DescriptorBufferInfo(vtAccelerator->_bvhBlockUniform->_descriptorInfo())),
                };
                vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
            };
        };

        return result;
    };


};
