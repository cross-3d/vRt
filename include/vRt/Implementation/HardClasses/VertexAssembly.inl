#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vrt;

    inline VtResult createVertexAssemblyPipeline(std::shared_ptr<Device> _vtDevice, const VtVertexAssemblyPipelineCreateInfo& info, std::shared_ptr<VertexAssemblyPipeline>& vtVertexAssembly) {
        VtResult result = VK_SUCCESS;
        auto vkDevice = _vtDevice->_device;
        auto vkPipelineCache = _vtDevice->_pipelineCache;
        //auto vtVertexAssembly = (_vtVertexAssembly = std::make_shared<VertexAssemblyPipeline>());
        vtVertexAssembly = std::make_shared<VertexAssemblyPipeline>();
        vtVertexAssembly->_device = _vtDevice;
        vtVertexAssembly->_pipelineLayout = info.pipelineLayout._vtPipelineLayout;
        vtVertexAssembly->_vertexAssemblyPipeline = createCompute(vkDevice, info.vertexAssemblyModule, *vtVertexAssembly->_pipelineLayout, vkPipelineCache);
        return result;
    };

    inline VtResult createVertexAssemblySet(std::shared_ptr<Device> _vtDevice, const VtVertexAssemblySetCreateInfo &info, std::shared_ptr<VertexAssemblySet>& vtVertexAssembly) {
        VtResult result = VK_SUCCESS;
        //auto vtVertexAssembly = (_vtVertexAssembly = std::make_shared<VertexAssemblySet>());
        auto vkDevice = _vtDevice->_device;
        vtVertexAssembly = std::make_shared<VertexAssemblySet>();
        vtVertexAssembly->_device = _vtDevice;

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
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_bitfieldBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_SINT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_materialBuffer);

            // accelerate normal calculation by storing of
            bfi.bufferSize = maxPrimitives * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_normalBuffer);

            bfi.bufferSize = maxPrimitives * 3ull * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_verticeBuffer);

            bfi.bufferSize = maxPrimitives * 3ull * sizeof(float) * 4ull; // restored due fatal issue
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_verticeBufferSide);

            bfi.bufferSize = sizeof(uint32_t) * 4ull;
            bfi.format = VK_FORMAT_R32_UINT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_countersBuffer);

            // create vertex attribute buffer
            VtDeviceImageCreateInfo tfi = {};
            tfi.familyIndex = _vtDevice->_mainFamilyIndex;
            tfi.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            tfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            tfi.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
            tfi.layout = VK_IMAGE_LAYOUT_GENERAL;
            tfi.mipLevels = 1;
            //tfi.size = { uint32_t(aWidth), uint32_t(tiled(maxPrimitives * 3ull * ATTRIB_EXTENT, aWidth) + 1ull), 1u };
            tfi.size = { uint32_t(aWidth), uint32_t(tiled(maxPrimitives * 4ull * ATTRIB_EXTENT, aWidth) + 1ull), 1u };
            createDeviceImage(_vtDevice, tfi, vtVertexAssembly->_attributeTexelBuffer);
        };

        { // create desciptor set
            std::vector<vk::PushConstantRange> constRanges = {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(12))
            };
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
            };
            auto dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtVertexAssembly->_descriptorSet = dsc[0];


            vk::Sampler attributeSampler = vk::Device(vkDevice).createSampler(vk::SamplerCreateInfo()
                .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                .setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest).setAddressModeU(vk::SamplerAddressMode::eRepeat)
                //.setMagFilter(vk::Filter::eLinear ).setMinFilter(vk::Filter::eLinear ).setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setUnnormalizedCoordinates(VK_TRUE)
            );
            auto writeTmpl = vk::WriteDescriptorSet(vtVertexAssembly->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            auto attrbView = vk::DescriptorImageInfo(vtVertexAssembly->_attributeTexelBuffer->_descriptorInfo()).setSampler(attributeSampler);

            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo((vk::DescriptorBufferInfo*)&vtVertexAssembly->_countersBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&vtVertexAssembly->_materialBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&vtVertexAssembly->_bitfieldBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&vtVertexAssembly->_verticeBuffer->_bufferView),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setDescriptorType(vk::DescriptorType::eStorageImage).setPImageInfo((vk::DescriptorImageInfo*)&vtVertexAssembly->_attributeTexelBuffer->_descriptorInfo()),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(5).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&vtVertexAssembly->_verticeBufferSide->_bufferView), // planned to replace
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&attrbView),
                vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&vtVertexAssembly->_normalBuffer->_bufferView),
            };
            vk::Device(vkDevice).updateDescriptorSets(writes, {});
        };

        return result;
    };
};
