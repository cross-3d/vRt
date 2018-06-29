#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;


    inline VtResult createPipelineLayout(std::shared_ptr<Device> _vtDevice, VkPipelineLayoutCreateInfo vtPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout, VtPipelineLayoutType type) {
        VtResult result = VK_SUCCESS;

        auto& vtPipelineLayout = (_vtPipelineLayout = std::make_shared<PipelineLayout>());
        vtPipelineLayout->_device = _vtDevice;
        vtPipelineLayout->_type = type;

        auto dsLayouts = type == VT_PIPELINE_LAYOUT_TYPE_RAYTRACING ?
            std::vector<vk::DescriptorSetLayout>{
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["materialSet"]),
            } : 
            std::vector<vk::DescriptorSetLayout>{
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
            };

        for (int i = 0; i < vtPipelineLayoutCreateInfo.setLayoutCount; i++) {
            dsLayouts.push_back(vtPipelineLayoutCreateInfo.pSetLayouts[i]);
        }

        vtPipelineLayoutCreateInfo.setLayoutCount = dsLayouts.size();
        vtPipelineLayoutCreateInfo.pSetLayouts = (VkDescriptorSetLayout*)(dsLayouts.data());

        VkPushConstantRange rng = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, strided<uint32_t>(1));
        if (type != VT_PIPELINE_LAYOUT_TYPE_RAYTRACING) {
            vtPipelineLayoutCreateInfo.pPushConstantRanges = &rng;
            vtPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        };

        vtPipelineLayout->_pipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo(vtPipelineLayoutCreateInfo));
        return result;
    };

};
