#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;


    inline VtResult createRayTracingPipelineLayout(std::shared_ptr<Device> _vtDevice, VkPipelineLayoutCreateInfo vtRayTracingPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& _vtPipelineLayout) {
        VtResult result = VK_SUCCESS;

        auto& vtPipelineLayout = (_vtPipelineLayout = std::make_shared<PipelineLayout>());
        vtPipelineLayout->_device = _vtDevice;

        std::vector<vk::DescriptorSetLayout> dsLayouts = {
            vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]),
            vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["materialSet"])
        };

        for (int i = 0; i < vtRayTracingPipelineLayoutCreateInfo.setLayoutCount; i++) {
            dsLayouts.push_back(vtRayTracingPipelineLayoutCreateInfo.pSetLayouts[i]);
        }

        vtRayTracingPipelineLayoutCreateInfo.setLayoutCount = dsLayouts.size();
        vtRayTracingPipelineLayoutCreateInfo.pSetLayouts = (VkDescriptorSetLayout*)(dsLayouts.data());

        vtPipelineLayout->_pipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo(vtRayTracingPipelineLayoutCreateInfo));

        return result;
    };

};
