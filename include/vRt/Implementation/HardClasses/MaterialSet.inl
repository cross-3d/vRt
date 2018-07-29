#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // ray tracing set of state
    static inline VtResult createMaterialSet(std::shared_ptr<Device> _vtDevice, const VtMaterialSetCreateInfo& info, std::shared_ptr<MaterialSet>& _vtMaterialSet) {
        VtResult result = VK_SUCCESS;

        auto& vtMaterialSet = (_vtMaterialSet = std::make_shared<MaterialSet>());
        auto vkDevice = _vtDevice->_device;
        vtMaterialSet->_device = _vtDevice;

        // planned variable size
        {
            vtMaterialSet->_materialCount = info.materialCount;

            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

                bfi.bufferSize = 8ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtMaterialSet->_constBuffer);
            };

            { // planned to add support of default element in not enough 
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["materialSet"]),
                };
                auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtMaterialSet->_descriptorSet = dsc[0];

                std::vector<vk::DescriptorImageInfo> _samplers = {}, _images = {};
                const auto samplerCount = std::min(info.samplerCount, 16u), imageCount = std::min(info.imageCount, 64u);

                // fill as many as possible
                for (uint32_t i = 0; i < samplerCount; i++) { _samplers.push_back(vk::DescriptorImageInfo().setSampler(info.pSamplers[i])); }
                for (uint32_t i = 0; i < imageCount; i++) { _images.push_back(vk::DescriptorImageInfo(info.pImages[i])); }

                // autofill for avoid validation errors
                for (uint32_t i = samplerCount; i < 16; i++) { _samplers.push_back(vk::DescriptorImageInfo().setSampler(info.pSamplers[samplerCount-1])); }
                for (uint32_t i = imageCount; i < 64; i++) { _images.push_back(vk::DescriptorImageInfo(info.pImages[imageCount-1])); }

                
                auto _write_tmpl = vk::WriteDescriptorSet(vtMaterialSet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(_images.size()).setPImageInfo(_images.data()),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eSampler).setDescriptorCount(_samplers.size()).setPImageInfo(_samplers.data()),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(bufferDescriptorInfo(info.bMaterialDescriptionsBuffer))),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(bufferDescriptorInfo(info.bImageSamplerCombinations))),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtMaterialSet->_constBuffer->_descriptorInfo())),
                };
                vk::Device(vkDevice).updateDescriptorSets(writes, {});
            };
        }

        return result;
    }
};
