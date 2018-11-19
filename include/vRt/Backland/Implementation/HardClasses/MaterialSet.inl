#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;


    MaterialSet::~MaterialSet() {
        if (_descriptorSet) vk::Device(VkDevice(*_device)).freeDescriptorSets(_device->_descriptorPool, { vk::DescriptorSet(_descriptorSet) });
        _descriptorSet = {};
    };

    // ray tracing set of state
    VtResult createMaterialSet(std::shared_ptr<Device> _vtDevice,  VtMaterialSetCreateInfo info, std::shared_ptr<MaterialSet>& vtMaterialSet) {
        VtResult result = VK_SUCCESS;

        //auto vtMaterialSet = (_vtMaterialSet = std::make_shared<MaterialSet>());
        auto vkDevice = _vtDevice->_device;
        vtMaterialSet = std::make_shared<MaterialSet>();
        vtMaterialSet->_device = _vtDevice;

        // planned variable size
        {
            vtMaterialSet->_materialCount = info.materialCount;

            {
                VtDeviceBufferCreateInfo bfi = {};
                bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

                bfi.bufferSize = 8ull * sizeof(uint32_t);
                bfi.format = VK_FORMAT_UNDEFINED;
                createDeviceBuffer(_vtDevice, bfi, vtMaterialSet->_constBuffer);
            };

            { // planned to add support of default element in not enough 
                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["materialSet"]),
                };
                const auto&& dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                vtMaterialSet->_descriptorSet = std::move(dsc[0]);

                std::vector<vk::DescriptorImageInfo> _samplers = {}, _images = {};
                const auto samplerCount = std::min(info.samplerCount, uint32_t(VRT_MAX_SAMPLERS)), imageCount = std::min(info.imageCount, uint32_t(VRT_MAX_IMAGES));

                // fill as many as possible
                for (auto i = 0u; i < samplerCount; i++) { _samplers.push_back(vk::DescriptorImageInfo().setSampler(info.pSamplers[i])); }
                for (auto i = 0u; i < imageCount; i++) { _images.push_back(vk::DescriptorImageInfo(info.pImages[i])); }

                // autofill for avoid validation errors
                for (auto i = samplerCount; i < VRT_MAX_SAMPLERS; i++) { _samplers.push_back(vk::DescriptorImageInfo().setSampler(info.pSamplers[samplerCount-1])); }
                for (auto i = imageCount; i < VRT_MAX_IMAGES; i++) { _images.push_back(vk::DescriptorImageInfo(info.pImages[imageCount-1])); }

                const auto bView = vk::Device(*_vtDevice).createBufferView(vk::BufferViewCreateInfo().setFormat(vk::Format::eR16G16Uint).setBuffer(info.bImageSamplerCombinations).setOffset(0).setRange(VK_WHOLE_SIZE));
                const auto matDescBuf = bufferDescriptorInfo(info.bMaterialDescriptionsBuffer), imgCompBuf = bufferDescriptorInfo(info.bImageSamplerCombinations);
                const auto writeTmpl = vk::WriteDescriptorSet(vtMaterialSet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(_images.size()).setPImageInfo(_images.data()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eSampler).setDescriptorCount(_samplers.size()).setPImageInfo(_samplers.data()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&matDescBuf),
                    //vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo((vk::DescriptorBufferInfo*)&imgCompBuf),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&bView),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setPBufferInfo((vk::DescriptorBufferInfo*)&vtMaterialSet->_constBuffer->_descriptorInfo()),
                };
                vk::Device(vkDevice).updateDescriptorSets(writes, {});
            };
        }

        return result;
    }
};
