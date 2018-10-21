#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;

    VtResult createPipelineLayout(std::shared_ptr<Device> _vtDevice, VtPipelineLayoutCreateInfo vtPipelineLayoutCreateInfo, std::shared_ptr<PipelineLayout>& vtPipelineLayout, VtPipelineLayoutType type) {
        VtResult result = VK_SUCCESS;
        auto vkDevice = _vtDevice->_device;

        auto vkPipelineLayout = vtPipelineLayoutCreateInfo.pGeneralPipelineLayout ? vk::PipelineLayoutCreateInfo(*vtPipelineLayoutCreateInfo.pGeneralPipelineLayout) : vk::PipelineLayoutCreateInfo();
        //auto vtPipelineLayout = (_vtPipelineLayout = std::make_shared<PipelineLayout>());
        vtPipelineLayout = std::make_shared<PipelineLayout>();
        vtPipelineLayout->_device = _vtDevice;
        vtPipelineLayout->_type = type;

        std::vector<vk::DescriptorSetLayout> dsLayouts = {vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap[type == VT_PIPELINE_LAYOUT_TYPE_RAYTRACING ? "rayTracing" : "empty"])};
        if (type == VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT) {
            dsLayouts.push_back(vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]));
            dsLayouts.push_back(vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]));
        } else 
        if (type == VT_PIPELINE_LAYOUT_TYPE_RAYTRACING && vtPipelineLayoutCreateInfo.enableMaterialSet) {
            dsLayouts.push_back(_vtDevice->_descriptorLayoutMap["materialSet"]);
        };

        for (auto i = 0; i < vkPipelineLayout.setLayoutCount; i++) {
            dsLayouts.push_back(vkPipelineLayout.pSetLayouts[i]);
        };

        vkPipelineLayout.setLayoutCount = dsLayouts.size();
        vkPipelineLayout.pSetLayouts = dsLayouts.data();

        const auto rng = vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, strided<uint32_t>(1));
        if (type != VT_PIPELINE_LAYOUT_TYPE_RAYTRACING) {
            vkPipelineLayout.pPushConstantRanges = &rng;
            vkPipelineLayout.pushConstantRangeCount = 1;
        };

        vtPipelineLayout->_vsLayout = vk::Device(vkDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo(vkPipelineLayout));
        vtPipelineLayout->_rtLayout = vtPipelineLayout->_vsLayout;
        if (type == VT_PIPELINE_LAYOUT_TYPE_VERTEXINPUT) {
            dsLayouts[0] = vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["rayTracing"]); // attemp to shift descriptor sets 
            dsLayouts[1] = vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["hlbvh2"]), vkPipelineLayout.pSetLayouts = dsLayouts.data(); // replace inputs to hlbvh2 data (require for ray-tracing)
            vtPipelineLayout->_rtLayout = vk::Device(vkDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo(vkPipelineLayout));
        };
        return result;
    };

};
