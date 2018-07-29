#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;

    static inline VtResult createPipelineLayout(std::shared_ptr<Device> _vtDevice, VtPipelineLayoutCreateInfo vtPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout, VtPipelineLayoutType type) {
        VtResult result = VK_SUCCESS;
        auto vkDevice = _vtDevice->_device;

        auto vkPipelineLayout = vtPipelineLayoutCreateInfo.pGeneralPipelineLayout ? vk::PipelineLayoutCreateInfo(*vtPipelineLayoutCreateInfo.pGeneralPipelineLayout) : vk::PipelineLayoutCreateInfo();

        auto& vtPipelineLayout = (_vtPipelineLayout = std::make_shared<PipelineLayout>());
        vtPipelineLayout->_device = _vtDevice;
        vtPipelineLayout->_type = type;

        auto dsLayouts = type == VT_PIPELINE_LAYOUT_TYPE_RAYTRACING ?
            std::vector<vk::DescriptorSetLayout>{
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
            } : 
            std::vector<vk::DescriptorSetLayout>{
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
            };

        if (vtPipelineLayoutCreateInfo.enableMaterialSet && type == VT_PIPELINE_LAYOUT_TYPE_RAYTRACING) {
            dsLayouts.push_back(_vtDevice->_descriptorLayoutMap["materialSet"]);
        }

        for (uint32_t i = 0; i < vkPipelineLayout.setLayoutCount; i++) {
            dsLayouts.push_back(vkPipelineLayout.pSetLayouts[i]);
        }

        vkPipelineLayout.setLayoutCount = dsLayouts.size();
        vkPipelineLayout.pSetLayouts = dsLayouts.data();

        auto rng = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, strided<uint32_t>(1));
        if (type != VT_PIPELINE_LAYOUT_TYPE_RAYTRACING) {
            vkPipelineLayout.pPushConstantRanges = &rng;
            vkPipelineLayout.pushConstantRangeCount = 1;
        };

        vtPipelineLayout->_pipelineLayout = vk::Device(vkDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo(vkPipelineLayout));
        return result;
    };

};
