#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    // ray tracing set of state
    inline VtResult createMaterialSet(std::shared_ptr<Device> _vtDevice, const VtMaterialSetCreateInfo& info, std::shared_ptr<MaterialSet>& _vtMaterialSet) {
        VtResult result = VK_SUCCESS;

        auto& vtMaterialSet = (_vtMaterialSet = std::make_shared<MaterialSet>());
        vtMaterialSet->_device = _vtDevice;

        // planned variable size
        auto rayCount = 4096 * 4096;
        {
            vtMaterialSet->_materialCount = info.materialCount;

            {
                VtDeviceBufferCreateInfo bfi;
                bfi.familyIndex = _vtDevice->_mainFamilyIndex;
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

                /*
                bfi.bufferSize = info.imageSamplerCount * sizeof(uint64_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtMaterialSet->_virtualSamplerCombinedBuffer);
                */

                bfi.bufferSize = 8 * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtMaterialSet->_constBuffer);
            };



            { // planned to add support of default element in not enough 
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["materialSet"]),
                };
                auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtMaterialSet->_descriptorSet = dsc[0];


                std::vector<vk::DescriptorImageInfo> _samplers = {};
                std::vector<vk::DescriptorImageInfo> _images = {};
                
                // fill as many as possible
                const auto samplerCount = std::min(info.samplerCount, 16u), imageCount = std::min(info.imageCount, 64u);
                for (int i = 0; i < samplerCount; i++) { _samplers.push_back(vk::DescriptorImageInfo().setSampler(info.pSamplers[i])); }
                for (int i = 0; i < imageCount; i++) { _images.push_back(vk::DescriptorImageInfo(info.pImages[i])); }

                // autofill for avoid validation errors
                for (int i = info.samplerCount; i < 16; i++) { _samplers.push_back(vk::DescriptorImageInfo().setSampler(info.pSamplers[info.samplerCount-1])); }
                for (int i = info.imageCount; i < 64; i++) { _images.push_back(vk::DescriptorImageInfo(info.pImages[info.imageCount-1])); }

                
                auto _write_tmpl = vk::WriteDescriptorSet(vtMaterialSet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(_images.size()).setPImageInfo(_images.data()),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eSampler).setDescriptorCount(_samplers.size()).setPImageInfo(_samplers.data()),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(bufferDescriptorInfo(info.bMaterialDescriptionsBuffer))),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(bufferDescriptorInfo(info.bImageSamplerCombinations))),
                    vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtMaterialSet->_constBuffer->_descriptorInfo())),
                };
                vk::Device(*_vtDevice).updateDescriptorSets(writes, {});
            };
        }


        return result;
    }

};
