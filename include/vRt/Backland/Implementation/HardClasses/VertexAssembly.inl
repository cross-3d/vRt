#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;

    VtResult createAssemblyPipeline(std::shared_ptr<Device> _vtDevice, VtAttributePipelineCreateInfo info, std::shared_ptr<AssemblyPipeline>& assemblyPipeline, const bool native) {
        VtResult result = VK_SUCCESS;
        auto vkDevice = _vtDevice->_device;
        auto vkPipelineCache = _vtDevice->_pipelineCache;
        assemblyPipeline = std::make_shared<AssemblyPipeline>();
        assemblyPipeline->_device = _vtDevice;
        assemblyPipeline->_pipelineLayout = info.pipelineLayout;
        assemblyPipeline->_vkPipeline = createCompute(vkDevice, info.assemblyModule, *assemblyPipeline->_pipelineLayout, vkPipelineCache);
        return result;
    };

    VtResult createVertexAssemblySet(std::shared_ptr<Device> _vtDevice, VtVertexAssemblySetCreateInfo info, std::shared_ptr<VertexAssemblySet>& assemblyPipeline) {
        VtResult result = VK_SUCCESS;
        //auto assemblyPipeline = (_assemblyPipeline = std::make_shared<VertexAssemblySet>());
        auto vkDevice = _vtDevice->_device;
        assemblyPipeline = std::make_shared<VertexAssemblySet>();
        assemblyPipeline->_device = _vtDevice;

        const auto maxPrimitives = info.maxPrimitives;
        constexpr const auto aWidth = 4096ull * 3ull;

        // build vertex input assembly program
        {
            VtDeviceBufferCreateInfo bfi = {};
            bfi.familyIndex = _vtDevice->_mainFamilyIndex;
            bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

            // vertex data buffers
            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createDeviceBuffer(_vtDevice, bfi, assemblyPipeline->_bitfieldBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            createDeviceBuffer(_vtDevice, bfi, assemblyPipeline->_materialBuffer);

            // accelerate normal calculation by storing of
            bfi.bufferSize = maxPrimitives * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, assemblyPipeline->_normalBuffer);

            bfi.bufferSize = maxPrimitives * 3ull * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, assemblyPipeline->_verticeBuffer);

            bfi.bufferSize = maxPrimitives * 3ull * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, assemblyPipeline->_verticeBufferSide);

            bfi.bufferSize = sizeof(uint32_t) * 8ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createDeviceBuffer(_vtDevice, bfi, assemblyPipeline->_countersBuffer);

            // create vertex attribute buffer
            VtDeviceImageCreateInfo tfi = {};
            tfi.familyIndex = _vtDevice->_mainFamilyIndex;
            tfi.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            tfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            tfi.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
            tfi.layout = VK_IMAGE_LAYOUT_GENERAL;
            tfi.mipLevels = 1;
            tfi.size = { uint32_t(aWidth), uint32_t(tiled(maxPrimitives * ATTRIB_EXTENT * 3ull, aWidth) + 1ull), 1u };
            createDeviceImage(_vtDevice, tfi, assemblyPipeline->_attributeTexelBuffer);
        };

        { // create desciptor set
            std::vector<vk::PushConstantRange> constRanges = {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(12))
            };
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
            };
            const auto&& dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            assemblyPipeline->_descriptorSet = std::move(dsc[0]);


            vk::Sampler attributeSampler = vk::Device(vkDevice).createSampler(vk::SamplerCreateInfo()
                .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                .setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest).setAddressModeU(vk::SamplerAddressMode::eRepeat)
                //.setMagFilter(vk::Filter::eLinear ).setMinFilter(vk::Filter::eLinear ).setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setUnnormalizedCoordinates(VK_TRUE)
            );
            const auto writeTmpl = vk::WriteDescriptorSet(assemblyPipeline->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            const auto attrbView = vk::DescriptorImageInfo(assemblyPipeline->_attributeTexelBuffer->_descriptorInfo()).setSampler(attributeSampler);

            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo((vk::DescriptorBufferInfo*)&assemblyPipeline->_countersBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&assemblyPipeline->_materialBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&assemblyPipeline->_bitfieldBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&assemblyPipeline->_verticeBuffer->_bufferView()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setDescriptorType(vk::DescriptorType::eStorageImage).setPImageInfo((vk::DescriptorImageInfo*)&assemblyPipeline->_attributeTexelBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(5).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&assemblyPipeline->_verticeBufferSide->_bufferView()), // planned to replace
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&attrbView),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&assemblyPipeline->_normalBuffer->_bufferView()),
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };

        return result;
    };
};
