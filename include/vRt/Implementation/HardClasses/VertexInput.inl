#pragma once

#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vt;
    


    inline VtResult createVertexAssembly(std::shared_ptr<Device> _vtDevice, const VtAcceleratorCreateInfo &info, std::shared_ptr<VertexAssembly>& _vtVertexAssembly) {
        VtResult result = VK_SUCCESS;
        auto& vtVertexAssembly = (_vtVertexAssembly = std::make_shared<VertexAssembly>());
        vtVertexAssembly->_device = _vtDevice;

        constexpr auto maxPrimitives = 1024u * 1024u; // planned import from descriptor

                                                      // build vertex input assembly program
        {
            constexpr auto ATTRIB_EXTENT = 4u; // no way to set more than it now

            VtDeviceBufferCreateInfo bfi;
            bfi.familyIndex = _vtDevice->_mainFamilyIndex;
            bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);

            // vertex data buffers
            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_UNDEFINED;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_orderBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_UNDEFINED;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_materialBuffer);

            bfi.bufferSize = maxPrimitives * sizeof(float) * 4;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_verticeBuffer);

            bfi.bufferSize = sizeof(uint32_t) * 4;
            bfi.format = VK_FORMAT_R32_UINT;
            createDeviceBuffer(_vtDevice, bfi, vtVertexAssembly->_countersBuffer);


            // create vertex attribute buffer
            VtDeviceImageCreateInfo tfi;
            tfi.familyIndex = _vtDevice->_mainFamilyIndex;
            tfi.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            tfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            tfi.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
            tfi.layout = VK_IMAGE_LAYOUT_GENERAL;
            tfi.mipLevels = 1;
            tfi.size = { 6144u, tiled(maxPrimitives * 3u * ATTRIB_EXTENT, 6144u) };
            createDeviceImage(_vtDevice, tfi, vtVertexAssembly->_attributeTexelBuffer);
        };

        {
            std::vector<vk::PushConstantRange> constRanges = {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(4))
            };
            std::vector<vk::DescriptorSetLayout> dsLayouts = {
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
            };
            auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            vtVertexAssembly->_vertexAssemblyDescriptorSet = dsc[0];


            vk::Sampler attributeSampler = vk::Device(*_vtDevice).createSampler(vk::SamplerCreateInfo()
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                .setMagFilter(vk::Filter::eNearest)
                .setMinFilter(vk::Filter::eNearest)
            );
            auto _write_tmpl = vk::WriteDescriptorSet(vtVertexAssembly->_vertexAssemblyDescriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
            std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBufferDynamic).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexAssembly->_countersBuffer->_descriptorInfo())),
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexAssembly->_materialBuffer->_descriptorInfo())),
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexAssembly->_orderBuffer->_descriptorInfo())),

                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&vk::BufferView(vtVertexAssembly->_verticeBuffer->_bufferView)),
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setDescriptorType(vk::DescriptorType::eStorageImage).setPImageInfo(&vk::DescriptorImageInfo(vtVertexAssembly->_attributeTexelBuffer->_descriptorInfo())),
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&vk::BufferView(vtVertexAssembly->_verticeBuffer->_bufferView)),
                vk::WriteDescriptorSet(_write_tmpl).setDstBinding(6).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&vk::DescriptorImageInfo(vtVertexAssembly->_attributeTexelBuffer->_descriptorInfo()).setSampler(attributeSampler)),
            };
            vk::Device(*_vtDevice).updateDescriptorSets(_write_tmpl, {});

            // create pipeline
            vtVertexAssembly->_vertexAssemblyPipelineLayout = vk::Device(*_vtDevice).createPipelineLayout(vk::PipelineLayoutCreateInfo({}, dsLayouts.size(), dsLayouts.data(), constRanges.size(), constRanges.data()));
            vtVertexAssembly->_vertexAssemblyPipeline = createCompute(VkDevice(*_vtDevice), _vtDevice->_shadersPath + "utils/vinput.comp.spv", vtVertexAssembly->_vertexAssemblyPipelineLayout, VkPipelineCache(*_vtDevice));
        };

        return result;
    };

    // TODO - add support for auto-creation of buffers in "VtVertexInputCreateInfo" from pointers and counts
    // also, planned to add support of offsets in buffers 
    inline VtResult createVertexInputSet(std::shared_ptr<Device> _vtDevice, VtVertexInputCreateInfo& info, std::shared_ptr<VertexInputSet>& _vtVertexInput) {
        VtResult result = VK_SUCCESS;
        auto& vtVertexInput = (_vtVertexInput = std::make_shared<VertexInputSet>());
        vtVertexInput->_device = _vtDevice;

        std::vector<vk::PushConstantRange> constRanges = {
            vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0u, strided<uint32_t>(4))
        };
        std::vector<vk::DescriptorSetLayout> dsLayouts = {
            vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
        };
        auto dsc = vk::Device(*_vtDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
        vtVertexInput->_descriptorSet = dsc[0];

        // 
        VtDeviceBufferCreateInfo bfi;
        bfi.familyIndex = _vtDevice->_mainFamilyIndex;
        bfi.usageFlag = VkBufferUsageFlags(vk::BufferUsageFlagBits::eStorageBuffer);
        bfi.bufferSize = sizeof(uint32_t) * 8;
        bfi.format = VK_FORMAT_UNDEFINED;

        // planned add external buffer support
        createDeviceBuffer(_vtDevice, bfi, vtVertexInput->_uniformBlockBuffer);
        _vtDevice->_deviceBuffersPtrs.push_back(vtVertexInput->_uniformBlockBuffer); // pin buffer with device

                                                                                     // set primitive count (will loaded to "_uniformBlockBuffer" by cmdUpdateBuffer)
        vtVertexInput->_uniformBlock.primitiveCount = info.primitiveCount;
        vtVertexInput->_uniformBlock.verticeAccessor = info.verticeAccessor;
        vtVertexInput->_uniformBlock.indiceAccessor = info.indiceAccessor;
        vtVertexInput->_uniformBlock.materialID = info.materialID;

        // write descriptors
        auto _write_tmpl = vk::WriteDescriptorSet(vtVertexInput->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&vk::BufferView(vtVertexInput->_dataSourceBuffer->_bufferView)),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferRegionBindings->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferViews->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(3).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferAccessors->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(4).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_bBufferAttributeBindings->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(5).setPBufferInfo(&vk::DescriptorBufferInfo(vtVertexInput->_uniformBlockBuffer->_descriptorInfo())),
        };
        vk::Device(*_vtDevice).updateDescriptorSets(_write_tmpl, {});

        return result;
    };



};
