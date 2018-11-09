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

        // 
        auto originalAssembler = _vtDevice->_nativeVertexAssembler[0];
        if (originalAssembler) {
            assemblyPipeline->_pipelineLayout = _vtDevice->_nativeVertexAssembler[0]->_pipelineLayout;
            assemblyPipeline->_intrpPipeline = _vtDevice->_nativeVertexAssembler[0]->_intrpPipeline;
            assemblyPipeline->_inputPipeline = _vtDevice->_nativeVertexAssembler[0]->_inputPipeline;
        };
        if (info.pipelineLayout) assemblyPipeline->_pipelineLayout = info.pipelineLayout;
        if (info.assemblyModule.module) assemblyPipeline->_inputPipeline = createCompute(vkDevice, info.assemblyModule, assemblyPipeline->_pipelineLayout->_vsLayout, vkPipelineCache);
        if (info.interpolModule.module) assemblyPipeline->_intrpPipeline = createCompute(vkDevice, info.interpolModule, assemblyPipeline->_pipelineLayout->_rtLayout, vkPipelineCache);
        return result;
    };

    VertexAssemblySet::~VertexAssemblySet() {
        _descriptorSetGenerator = {};
        if (_descriptorSet) vk::Device(VkDevice(*_device)).freeDescriptorSets(_device->_descriptorPool, { vk::DescriptorSet(_descriptorSet) });
        _descriptorSet = {};
    };

    VtResult createVertexAssemblySet(std::shared_ptr<Device> _vtDevice, VtVertexAssemblySetCreateInfo info, std::shared_ptr<VertexAssemblySet>& _assemblySet) {
        VtResult result = VK_SUCCESS;
        _assemblySet = std::make_shared<VertexAssemblySet>();
        _assemblySet->_device = _vtDevice;
        _assemblySet->_vertexAssembly = _vtDevice->_nativeVertexAssembler[0];
        if (info.vertexAssembly) _assemblySet->_vertexAssembly = info.vertexAssembly;

                  const auto maxPrimitives = info.maxPrimitives;
        constexpr const auto aWidth = 4096ull * 3ull;

        // build vertex input assembly program
        {
            // planned import from descriptor
            std::shared_ptr<BufferManager> bManager;
            createBufferManager(_vtDevice, bManager);


            VtBufferRegionCreateInfo bfi = {};

            // vertex data buffers
            bfi.bufferSize = maxPrimitives * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            bfi.offset = 0ull;

            // bitfield data per vertex node
            if (info.sharedBitfieldBuffer) { bfi.offset = info.sharedBitfieldOffset; createBufferRegion(info.sharedBitfieldBuffer, bfi, _assemblySet->_bitfieldBuffer, _vtDevice); } else 
            { createBufferRegion(bManager, bfi, _assemblySet->_bitfieldBuffer); };

            // material indexing buffer 
            bfi.offset = 0ull;
            if (info.sharedMaterialIndexedBuffer) { bfi.offset = info.sharedMaterialIndexedOffset; createBufferRegion(info.sharedMaterialIndexedBuffer, bfi, _assemblySet->_materialBuffer, _vtDevice); } else 
            { createBufferRegion(bManager, bfi, _assemblySet->_materialBuffer); };

            // accelerate normal calculation by storing of
            bfi.bufferSize = maxPrimitives * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            bfi.offset = 0ull;
            if (info.sharedNormalBuffer) { bfi.offset = info.sharedNormalOffset; createBufferRegion(info.sharedNormalBuffer, bfi, _assemblySet->_normalBuffer, _vtDevice); } else 
            { createBufferRegion(bManager, bfi, _assemblySet->_normalBuffer); };

            // vertex data in use
            bfi.bufferSize = maxPrimitives * 3ull * sizeof(float) * 4ull;
            bfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            bfi.offset = 0ull;
            if (info.sharedVertexInUseBuffer) { bfi.offset = info.sharedVertexInUseOffset; createBufferRegion(info.sharedVertexInUseBuffer, bfi, _assemblySet->_verticeBufferInUse, _vtDevice); } else 
            { createBufferRegion(bManager, bfi, _assemblySet->_verticeBufferInUse); };
             
            // vertex data for caching
            bfi.offset = 0ull;
            if (info.sharedVertexCacheBuffer) { bfi.offset = info.sharedVertexCacheOffset; createBufferRegion(info.sharedVertexCacheBuffer, bfi, _assemblySet->_verticeBufferCached, _vtDevice); } else 
            { createBufferRegion(bManager, bfi, _assemblySet->_verticeBufferCached); };

            // 
            bfi.bufferSize = 4ull * 3ull * sizeof(uint32_t);//maxPrimitives * 3ull * sizeof(uint32_t);
            bfi.format = VK_FORMAT_R32_UINT;
            bfi.offset = 0ull;
            { createBufferRegion(bManager, bfi, _assemblySet->_indexBuffer); };

            // counters for assembling ( no using in traversing or interpolation )
            bfi.bufferSize = sizeof(uint32_t) * 8ull;
            bfi.format = VK_FORMAT_R32_UINT;
            bfi.offset = 0ull;
            { createBufferRegion(bManager, bfi, _assemblySet->_countersBuffer); };

            { // build final shared buffer for this class
                VtDeviceBufferCreateInfo bfic = {};
                bfic.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                createSharedBuffer(bManager, bfic, _assemblySet->_sharedBuffer);
            };

            // create vertex attribute buffer
            if (!info.sharedAttributeImageDescriptor.imageView) {
                VtDeviceImageCreateInfo tfi = {};
                tfi.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                tfi.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                tfi.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
                tfi.layout = VK_IMAGE_LAYOUT_GENERAL;
                tfi.mipLevels = 1;
                tfi.size = { uint32_t(aWidth), uint32_t(tiled(maxPrimitives * ATTRIB_EXTENT * 3ull, aWidth) + 1ull), 1u };
                createDeviceImage(_vtDevice, tfi, _assemblySet->_attributeTexelBuffer);
            };
        };

        _assemblySet->_descriptorSetGenerator = [=]() { // create caller for generate descriptor set
            if (!_assemblySet->_descriptorSet) { // create desciptor set
                auto vkDevice = _vtDevice->_device;

                std::vector<vk::DescriptorSetLayout> dsLayouts = {
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexData"]),
                    vk::DescriptorSetLayout(_vtDevice->_descriptorLayoutMap["vertexInputSet"]),
                };
                const auto&& dsc = vk::Device(vkDevice).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(_vtDevice->_descriptorPool).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
                _assemblySet->_descriptorSet = std::move(dsc[0]);

                vk::Sampler attributeSampler = vk::Device(vkDevice).createSampler(vk::SamplerCreateInfo()
                    .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                    .setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest).setAddressModeU(vk::SamplerAddressMode::eRepeat)
                    //.setMagFilter(vk::Filter::eLinear ).setMinFilter(vk::Filter::eLinear ).setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setUnnormalizedCoordinates(VK_TRUE)
                );

                const auto writeTmpl = vk::WriteDescriptorSet(_assemblySet->_descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
                const auto attrbView = vk::DescriptorImageInfo(info.sharedAttributeImageDescriptor.imageView ? info.sharedAttributeImageDescriptor : _assemblySet->_attributeTexelBuffer->_descriptorInfo()).setSampler(attributeSampler);
                std::vector<vk::WriteDescriptorSet> writes = {
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo((vk::DescriptorBufferInfo*)&_assemblySet->_countersBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(1).setPBufferInfo((vk::DescriptorBufferInfo*)&_assemblySet->_materialBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo((vk::DescriptorBufferInfo*)&_assemblySet->_bitfieldBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&_assemblySet->_verticeBufferCached->_bufferView()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setDescriptorType(vk::DescriptorType::eStorageImage).setPImageInfo((vk::DescriptorImageInfo*)&_assemblySet->_attributeTexelBuffer->_descriptorInfo()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(5).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&_assemblySet->_verticeBufferInUse->_bufferView()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&attrbView),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&_assemblySet->_normalBuffer->_bufferView()),
                    vk::WriteDescriptorSet(writeTmpl).setDstBinding(8).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView((vk::BufferView*)&_assemblySet->_indexBuffer->_bufferView()),
                };
                vk::Device(vkDevice).updateDescriptorSets(writes, {});
            };
            _assemblySet->_descriptorSetGenerator = {};
        };


        // use extension
        if (_vtDevice->_hExtensionAccelerator.size() > 0 && _vtDevice->_hExtensionAccelerator[0]) {
            _vtDevice->_hExtensionAccelerator[0]->_ConstructVertexAssembly(_assemblySet);
        };


        return result;
    };
};
