
#include <appBase.hpp>




struct VtAppMaterial {
    glm::vec4 diffuse = glm::vec4(0.f);
    glm::vec4 specular = glm::vec4(0.f);
    glm::vec4 transmission = glm::vec4(0.f);
    glm::vec4 emissive = glm::vec4(0.f);

    float ior = 1.f;
    float roughness = 0.f;
    float alpharef = 0.f;
    float unk0f = 0.f;

    glm::uint diffuseTexture = 0;
    glm::uint specularTexture = 0;
    glm::uint bumpTexture = 0;
    glm::uint emissiveTexture = 0;

    int flags = 0;
    int alphafunc = 0;
    int binding = 0;
    int bitfield = 0;
};

struct VtCameraUniform {
    glm::mat4x4 camInv = glm::mat4x4(1.f);
    glm::mat4x4 projInv = glm::mat4x4(1.f);
    glm::vec4 sceneRes = glm::vec4(1.f);
    int enable360 = 0, r0 = 0, r1 = 0, r2 = 0;
};




// application fast utility for fill buffers
template<class T>
inline auto writeIntoBuffer(vte::Queue deviceQueue, const vt::VtDeviceBuffer& dBuffer, const std::vector<T>& vctr, size_t byteOffset = 0) {
    VkResult result = VK_SUCCESS;
    vt::vtSetBufferSubData<T>(vctr, deviceQueue->device->rtDev);
    vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
        vt::vtCmdCopyHostToDeviceBuffer(cmdBuf, deviceQueue->device->rtDev, dBuffer, 1, (const VkBufferCopy *)&(vk::BufferCopy(0, byteOffset, vte::strided<T>(vctr.size()))));
    });
    return result;
};

inline auto createBufferFast(vte::Queue deviceQueue, vt::VtDeviceBuffer& dBuffer, size_t byteSize = 1024 * 16) {
    vt::VtDeviceBufferCreateInfo dbs;
    dbs.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    dbs.bufferSize = 1024 * 16;
    dbs.familyIndex = deviceQueue->familyIndex;
    dbs.format = VK_FORMAT_R16_UINT;
    vt::vtCreateDeviceBuffer(deviceQueue->device->rtDev, &dbs, &dBuffer);
}







void main() {
    using namespace vt;

    if (!glfwInit()) exit(EXIT_FAILURE);
    if (!glfwVulkanSupported()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // choiced physical device
    uint32_t gpuID = 0; // at now "0" by default

    // create GLFW window
    std::string title = "vRt early test";
    uint32_t canvasWidth = 1280, canvasHeight = 720; float windowScale = 1.0;

    GLFWwindow* window = glfwCreateWindow(canvasWidth, canvasHeight, "vRt early test", NULL, NULL);
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }

    // create vulkan and ray tracing instance
    auto appfw = std::make_shared<vte::ApplicationBase>();
    auto instance = appfw->createInstance();

    // get physical devices
    auto physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.size() < 0) { glfwTerminate(); std::cerr << "Vulkan does not supported, or driver broken." << std::endl; exit(0); }

    // choice device
    if (gpuID >= physicalDevices.size()) { gpuID = physicalDevices.size() - 1; }
    if (gpuID < 0 || gpuID == -1) gpuID = 0;
    auto gpu = physicalDevices[gpuID];

    // create surface and get format by physical device
    glfwGetWindowContentScale(window, &windowScale, nullptr);
    appfw->createWindowSurface(window, canvasWidth, canvasHeight, title);
    appfw->format(appfw->getSurfaceFormat(gpu));

    // write physical device name
    vk::PhysicalDeviceProperties devProperties = gpu.getProperties();
    std::cout << "Current Device: ";
    std::cout << devProperties.deviceName << std::endl;

    // create combined device object
    const std::string shaderPack = "./shaders/amd/";
    auto deviceQueue = appfw->createDeviceQueue(gpu, false, shaderPack); // create default graphical device
    auto renderpass = appfw->createRenderpass(deviceQueue);


    // create image output
    VtDeviceImageCreateInfo dii;
    dii.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    dii.familyIndex = deviceQueue->familyIndex;
    dii.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
    dii.layout = VK_IMAGE_LAYOUT_GENERAL;
    dii.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    dii.size = { 1280, 720, 1 };
    VtDeviceImage outImage;
    vtCreateDeviceImage(deviceQueue->device->rtDev, &dii, &outImage);

    // dispatch image barrier
    vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
        vtCmdImageBarrier(cmdBuf, outImage);
    });




    //////////////////////////////////////
    // ray tracing preparing long stage //
    //////////////////////////////////////

    VtDeviceImage envImage;
    VtDeviceImage dullImage;
    VkSampler dullSampler;
    VtDeviceBuffer materialDescs;
    VtDeviceBuffer materialCombImages;
    VkDescriptorSet usrDescSet;
    std::vector<vk::DescriptorSetLayout> customedLayouts;
    VtMaterialSet materialSet;
    VtRayTracingSet raytracingSet;
    VtPipelineLayout rtPipelineLayout;
    VtPipeline rtPipeline;
    VtAcceleratorSet accelerator;
    VtVertexAssemblySet vertexAssembly;
    VtVertexInputSet vertexInput;
    VkCommandBuffer bCmdBuf, rtCmdBuf;
    VtDeviceBuffer rtUniformBuffer;


    // create vertex input buffer objects
    VtDeviceBuffer VDataSpace, VBufferRegions, VBufferView, VAccessorSet, VAttributes;
    {
        createBufferFast(deviceQueue, VDataSpace);
        createBufferFast(deviceQueue, VBufferRegions);
        createBufferFast(deviceQueue, VBufferView);
        createBufferFast(deviceQueue, VAccessorSet);
        createBufferFast(deviceQueue, VAttributes);
    }

    {
        // create dull image
        VtDeviceImageCreateInfo dii;
        dii.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        dii.familyIndex = deviceQueue->familyIndex;
        dii.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
        dii.layout = VK_IMAGE_LAYOUT_GENERAL;
        dii.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        dii.size = { 2, 2, 1 };
        vtCreateDeviceImage(deviceQueue->device->rtDev, &dii, &dullImage);

        // dispatch image barrier
        vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
            vtCmdImageBarrier(cmdBuf, dullImage);
        });
    }

    {
        // create dull sampler
        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.compareEnable = false;
        dullSampler = deviceQueue->device->logical.createSampler(samplerInfo); // create sampler
    }

    {
        // create dull material description set
        createBufferFast(deviceQueue, materialDescs, vte::strided<VtAppMaterial>(1));

        // set first buffer data
        VtAppMaterial initialMaterialDesc;
        writeIntoBuffer<VtAppMaterial>(deviceQueue, materialDescs, { initialMaterialDesc }, 0);
    }


    {
        // create dull material description set
        createBufferFast(deviceQueue, materialCombImages, vte::strided<VtVirtualCombinedImage>(1));

        // set first buffer data
        VtVirtualCombinedImage initialMaterialDesc;
        writeIntoBuffer<VtVirtualCombinedImage>(deviceQueue, materialCombImages, { initialMaterialDesc }, 0);
    }

    {
        // custom bindings for ray tracing systems
        const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // constants for generation shader
            vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute) // env map for miss shader
        };
        customedLayouts.push_back(deviceQueue->device->logical.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size())));

        // create descriptor set, based on layout
        // TODO: fill descriptor set by inputs
        auto dsc = deviceQueue->device->logical.allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(deviceQueue->device->descriptorPool).setPSetLayouts(customedLayouts.data()).setDescriptorSetCount(customedLayouts.size()));
        usrDescSet = dsc[0];
    }

    {
        // create env image
        VtDeviceImageCreateInfo dii;
        dii.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        dii.familyIndex = deviceQueue->familyIndex;
        dii.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
        dii.layout = VK_IMAGE_LAYOUT_GENERAL;
        dii.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        dii.size = { 2, 2, 1 };
        vtCreateDeviceImage(deviceQueue->device->rtDev, &dii, &envImage);

        // dispatch image barrier
        vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
            vtCmdImageBarrier(cmdBuf, envImage);
        });
    }

    {
        // initial matrices
        auto atMatrix = glm::lookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
        auto pjMatrix = glm::perspective(float(M_PI) / 4.f, 16.f / 9.f, 0.0001f, 1000.f);

        // create uniform buffer
        createBufferFast(deviceQueue, rtUniformBuffer, vte::strided<VtCameraUniform>(1));

        // set first uniform buffer data
        VtCameraUniform cameraUniformData;
        cameraUniformData.projInv = glm::transpose(glm::inverse(pjMatrix));
        cameraUniformData.camInv = glm::transpose(glm::inverse(atMatrix));
        cameraUniformData.sceneRes = glm::vec4(1280.f, 720.f, 1.f, 1.f);
        writeIntoBuffer<VtCameraUniform>(deviceQueue, rtUniformBuffer, { cameraUniformData }, 0);
    }

    { 
        // write ray tracing user defined descriptor set
        auto _write_tmpl = vk::WriteDescriptorSet(usrDescSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo(&vk::DescriptorBufferInfo(rtUniformBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&vk::DescriptorImageInfo(envImage->_descriptorInfo()).setSampler(dullSampler)),
        };
        vk::Device(deviceQueue->device->rtDev).updateDescriptorSets(writes, {});
    }

    {
        // create pipeline layout for ray tracing
        vk::PipelineLayoutCreateInfo vpi;
        vpi.pSetLayouts = customedLayouts.data();
        vpi.setLayoutCount = customedLayouts.size();
        vtCreateRayTracingPipelineLayout(deviceQueue->device->rtDev, &(VkPipelineLayoutCreateInfo)vpi, &rtPipelineLayout);
    }

    {
        // create ray tracing set
        VtRayTracingSetCreateInfo rtsi;
        vtCreateRayTracingSet(deviceQueue->device->rtDev, &rtsi, &raytracingSet);
    }

    { 
        // create material set
        VtMaterialSetCreateInfo mtsi;
        mtsi.imageCount = 1; mtsi.pImages = &dullImage->_descriptorInfo();
        mtsi.samplerCount = 1; mtsi.pSamplers = (VkSampler *)&dullSampler;
        mtsi.bMaterialDescriptionsBuffer = materialDescs;
        mtsi.bImageSamplerCombinations = materialCombImages;
        mtsi.materialCount = 1;
        vtCreateMaterialSet(deviceQueue->device->rtDev, &mtsi, &materialSet);
    }

    {
        // create ray tracing pipeline
        VtRayTracingPipelineCreateInfo rtpi;
        rtpi.closestModule = vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/closest-hit-shader.comp.spv"));
        rtpi.missModule = vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/miss-hit-shader.comp.spv"));
        rtpi.generationModule = vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/generation-shader.comp.spv"));
        rtpi.resolveModule = vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/resolve-shader.comp.spv"));
        rtpi.pipelineLayout = rtPipelineLayout;
        vtCreateRayTracingPipeline(deviceQueue->device->rtDev, &rtpi, &rtPipeline);
    }


    // create accelerator set
    VtAcceleratorSetCreateInfo acci;
    vtCreateAccelerator(deviceQueue->device->rtDev, &acci, &accelerator);


    // create vertex assembly
    VtVertexAssemblySetCreateInfo vtsi;
    vtCreateVertexAssembly(deviceQueue->device->rtDev, &vtsi, &vertexAssembly);



    {
        // all available accessors
        std::vector<VtVertexAccessor> accessors = {
            { 0, 0, VT_R32G32B32_SFLOAT }, // vertices
            { 1, 0, VT_R32G32B32_SFLOAT }, // normals
            { 2, 0, VT_R32G32B32_SFLOAT }, // texcoords
            { 3, 0, VT_R32G32B32_SFLOAT }, // tangents
            { 4, 0, VT_R32G32B32_SFLOAT }, // bitangents
        };
        writeIntoBuffer(deviceQueue, VAccessorSet, accessors, 0);

        // attribute binding with accessors
        std::vector<VtVertexAttributeBinding> attributes = {
            { 0, 1 }, 
            { 1, 2 }, 
            { 2, 3 }, 
            { 3, 4 },
        };
        writeIntoBuffer(deviceQueue, VAttributes, attributes, 0);

        // global buffer space
        std::vector<VtVertexAttributeBinding> bufferRegions = {
            { 0, 1024 * 16 },
        };
        writeIntoBuffer(deviceQueue, VBufferRegions, bufferRegions, 0);

        // vertice using first offsets for vertices, anothers using seros, planned add colors support
        std::vector<VtVertexBufferView> bufferViews = {
            { 0, 0, sizeof(glm::vec3) },
            { 0, sizeof(glm::vec3) * 6, sizeof(glm::vec3) },
            { 0, sizeof(glm::vec3) * 6, sizeof(glm::vec3) },
            { 0, sizeof(glm::vec3) * 6, sizeof(glm::vec3) },
            { 0, sizeof(glm::vec3) * 6, sizeof(glm::vec3) },
        };
        writeIntoBuffer(deviceQueue, VBufferView, bufferViews, 0);

        // test vertex data
        std::vector<glm::vec3> vertices = {
            glm::vec3( 1.f, -1.f, 0.f), 
            glm::vec3(-1.f, -1.f, 0.f), 
            glm::vec3( 0.f,  1.f, 0.f),

            glm::vec3( 1.f, -1.f, 0.f),
            glm::vec3(-1.f, -1.f, 0.f),
            glm::vec3( 0.f,  0.f, 1.f),
        };
        std::vector<glm::vec3> colors = {
            glm::vec3(1.f, 0.f, 0.f), 
            glm::vec3(0.f, 1.f, 0.f), 
            glm::vec3(0.f, 0.f, 1.f),

            glm::vec3(1.f, 0.f, 0.f),
            glm::vec3(0.f, 1.f, 0.f),
            glm::vec3(0.f, 0.f, 1.f),
        };
        std::vector<glm::vec3> zeros = {
            glm::vec3(0.f), 
            glm::vec3(0.f), 
            glm::vec3(0.f),
            
            glm::vec3(0.f),
            glm::vec3(0.f),
            glm::vec3(0.f),
        };

        // write data space by offsets
        writeIntoBuffer(deviceQueue, VDataSpace, vertices, 0u);
        writeIntoBuffer(deviceQueue, VDataSpace, zeros, sizeof(glm::vec3) * 6);
        writeIntoBuffer(deviceQueue, VDataSpace, colors, sizeof(glm::vec3) * 12);
    }

    // create vertex input
    {
        VtVertexInputCreateInfo vtii;
        vtii.topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;
        vtii.verticeAccessor = 0;
        vtii.indiceAccessor = -1;
        vtii.sourceBuffer = VDataSpace;
        vtii.bBufferAccessors = VAccessorSet;
        vtii.bBufferAttributeBindings = VAttributes;
        vtii.bBufferRegionBindings = VBufferRegions;
        vtii.bBufferViews = VBufferView;
        vtii.primitiveCount = 2;
        vtCreateVertexInputSet(deviceQueue->device->rtDev, &vtii, &vertexInput);
    }

    

    // ray tracing command buffers creation

    {
        // make accelerator and vertex builder command
        bCmdBuf = vte::createCommandBuffer(deviceQueue->device->rtDev, deviceQueue->commandPool, false);
        VtCommandBuffer qBCmdBuf; vtQueryCommandInterface(deviceQueue->device->rtDev, bCmdBuf, &qBCmdBuf);
        vtCmdBindAccelerator(qBCmdBuf, accelerator);
        vtCmdBindVertexAssembly(qBCmdBuf, vertexAssembly);
        vtCmdBindVertexInputSets(qBCmdBuf, 1, &vertexInput);
        vtCmdBuildVertexAssembly(qBCmdBuf);
        vtCmdBuildAccelerator(qBCmdBuf);
        vkEndCommandBuffer(qBCmdBuf);
    }

    {
        // make ray tracing command buffer
        rtCmdBuf = vte::createCommandBuffer(deviceQueue->device->rtDev, deviceQueue->commandPool, false);
        VtCommandBuffer qRtCmdBuf; vtQueryCommandInterface(deviceQueue->device->rtDev, rtCmdBuf, &qRtCmdBuf);
        vtCmdBindPipeline(qRtCmdBuf, VT_PIPELINE_BIND_POINT_RAY_TRACING, rtPipeline);
        vtCmdBindMaterialSet(qRtCmdBuf, VtEntryUsageFlags(VT_ENTRY_USAGE_CLOSEST | VT_ENTRY_USAGE_MISS), materialSet);
        vtCmdBindDescriptorSets(qRtCmdBuf, VT_PIPELINE_BIND_POINT_RAY_TRACING, rtPipelineLayout, 0, 1, &usrDescSet, 0, nullptr);
        vtCmdBindRayTracingSet(qRtCmdBuf, raytracingSet);
        vtCmdBindAccelerator(qRtCmdBuf, accelerator);
        vtCmdBindVertexAssembly(qRtCmdBuf, vertexAssembly);
        vtCmdDispatchRayTracing(qRtCmdBuf, 1280, 720);
        vkEndCommandBuffer(qRtCmdBuf);
    }

    //////////////////////////////////////
    // ray tracing preparing stage end  //
    //////////////////////////////////////



    // descriptor set bindings
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = { vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr) };
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = { deviceQueue->device->logical.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(descriptorSetLayoutBindings.data()).setBindingCount(1)) };

    // pipeline layout and cache
    auto pipelineLayout = deviceQueue->device->logical.createPipelineLayout(vk::PipelineLayoutCreateInfo().setPSetLayouts(descriptorSetLayouts.data()).setSetLayoutCount(descriptorSetLayouts.size()));
    auto descriptorSets = deviceQueue->device->logical.allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(deviceQueue->device->descriptorPool).setDescriptorSetCount(descriptorSetLayouts.size()).setPSetLayouts(descriptorSetLayouts.data()));

    // create pipeline
    vk::Pipeline trianglePipeline;
    {
        // pipeline stages
        std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStages = {
            vk::PipelineShaderStageCreateInfo().setModule(vte::loadAndCreateShaderModule(deviceQueue->device->logical, vte::readBinary(shaderPack + "/output/render.vert.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eVertex),
            vk::PipelineShaderStageCreateInfo().setModule(vte::loadAndCreateShaderModule(deviceQueue->device->logical, vte::readBinary(shaderPack + "/output/render.frag.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eFragment)
        };

        // blend modes per framebuffer targets
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments = {
            vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(true)
            .setSrcColorBlendFactor(vk::BlendFactor::eOne).setDstColorBlendFactor(vk::BlendFactor::eZero).setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne).setDstAlphaBlendFactor(vk::BlendFactor::eZero).setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorWriteMask(vk::ColorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        };

        // dynamic states
        std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        // create graphics pipeline
        trianglePipeline = deviceQueue->device->logical.createGraphicsPipeline(deviceQueue->device->pipelineCache, vk::GraphicsPipelineCreateInfo()
            .setPStages(pipelineShaderStages.data()).setStageCount(pipelineShaderStages.size())
            .setFlags(vk::PipelineCreateFlagBits::eAllowDerivatives)
            .setPVertexInputState(&vk::PipelineVertexInputStateCreateInfo())
            .setPInputAssemblyState(&vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleStrip))
            .setPViewportState(&vk::PipelineViewportStateCreateInfo().setViewportCount(1).setScissorCount(1))
            .setPRasterizationState(&vk::PipelineRasterizationStateCreateInfo()
                .setDepthClampEnable(false)
                .setRasterizerDiscardEnable(false)
                .setPolygonMode(vk::PolygonMode::eFill)
                .setCullMode(vk::CullModeFlagBits::eBack)
                .setFrontFace(vk::FrontFace::eCounterClockwise)
                .setDepthBiasEnable(false)
                .setDepthBiasConstantFactor(0)
                .setDepthBiasClamp(0)
                .setDepthBiasSlopeFactor(0)
                .setLineWidth(1.f))
            .setPDepthStencilState(&vk::PipelineDepthStencilStateCreateInfo()
                .setDepthTestEnable(false)
                .setDepthWriteEnable(false)
                .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                .setDepthBoundsTestEnable(false)
                .setStencilTestEnable(false))
            .setPColorBlendState(&vk::PipelineColorBlendStateCreateInfo()
                .setLogicOpEnable(false)
                .setLogicOp(vk::LogicOp::eClear)
                .setPAttachments(colorBlendAttachments.data())
                .setAttachmentCount(colorBlendAttachments.size()))
            .setLayout(pipelineLayout)
            .setRenderPass(renderpass)
            .setBasePipelineIndex(0)
            .setPMultisampleState(&vk::PipelineMultisampleStateCreateInfo().setRasterizationSamples(vk::SampleCountFlagBits::e1))
            .setPDynamicState(&vk::PipelineDynamicStateCreateInfo().setPDynamicStates(dynamicStates.data()).setDynamicStateCount(dynamicStates.size()))
            .setPTessellationState(&vk::PipelineTessellationStateCreateInfo())
            .setPNext(&vk::PipelineRasterizationConservativeStateCreateInfoEXT().setConservativeRasterizationMode(vk::ConservativeRasterizationModeEXT::eDisabled))
        );
    }


    { // write descriptors for showing texture
        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.compareEnable = false;
        auto sampler = deviceQueue->device->logical.createSampler(samplerInfo); // create sampler
        auto& image = outImage;

        // desc texture texture
        vk::DescriptorImageInfo imageDesc;
        imageDesc.imageLayout = vk::ImageLayout(image->_layout);
        imageDesc.imageView = vk::ImageView(image->_imageView);
        imageDesc.sampler = sampler;

        // update descriptors
        deviceQueue->device->logical.updateDescriptorSets(std::vector<vk::WriteDescriptorSet>{
            vk::WriteDescriptorSet().setDstSet(descriptorSets[0]).setDstBinding(0).setDstArrayElement(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&imageDesc),
        }, nullptr);
    }



    auto currentContext = std::make_shared<vte::GraphicsContext>();
    { // create graphic context
        auto& context = currentContext;

        // create graphics context
        context->queue = deviceQueue;
        context->pipeline = trianglePipeline;
        context->descriptorPool = deviceQueue->device->descriptorPool;
        context->descriptorSets = descriptorSets;
        context->pipelineLayout = pipelineLayout;

        // create framebuffers by size
        context->renderpass = renderpass;
        context->swapchain = appfw->createSwapchain(deviceQueue);
        context->framebuffers = appfw->createSwapchainFramebuffer(deviceQueue, context->swapchain, context->renderpass);
    }



    // rendering presentation 
    int32_t currSemaphore = -1; uint32_t currentBuffer = 0;
    auto tIdle = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(appfw->window())) {
        glfwPollEvents();


        { // reserved field for computing code



        }


        auto n_semaphore = currSemaphore;
        auto c_semaphore = (currSemaphore + 1) % currentContext->framebuffers.size();
        currSemaphore = c_semaphore;

        // acquire next image where will rendered (and get semaphore when will presented finally)
        n_semaphore = (n_semaphore >= 0 ? n_semaphore : (currentContext->framebuffers.size() - 1));
        currentContext->queue->device->logical.acquireNextImageKHR(currentContext->swapchain, std::numeric_limits<uint64_t>::max(), currentContext->framebuffers[n_semaphore].semaphore, nullptr, &currentBuffer);

        { // submit rendering (and wait presentation in device)
            // prepare viewport and clear info
            std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(std::array<float,4>{0.2f, 0.2f, 0.2f, 1.0f}), vk::ClearDepthStencilValue(1.0f, 0) };
            auto renderArea = vk::Rect2D(vk::Offset2D(0, 0), appfw->size());
            auto viewport = vk::Viewport(0.0f, 0.0f, appfw->size().width, appfw->size().height, 0, 1.0f);

            // create command buffer (with rewrite)
            auto& commandBuffer = (currentContext->framebuffers[n_semaphore].commandBuffer = vte::createCommandBuffer(currentContext->queue->device->logical, currentContext->queue->commandPool, false)); // do reference of cmd buffer
            commandBuffer.beginRenderPass(vk::RenderPassBeginInfo(currentContext->renderpass, currentContext->framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            commandBuffer.setViewport(0, std::vector<vk::Viewport> { viewport });
            commandBuffer.setScissor(0, std::vector<vk::Rect2D> { renderArea });
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, currentContext->pipeline);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, currentContext->pipelineLayout, 0, currentContext->descriptorSets, nullptr);
            commandBuffer.draw(4, 1, 0, 0);
            commandBuffer.endRenderPass();
            commandBuffer.end();

            // create render submission 
            std::vector<vk::Semaphore>
                waitSemaphores = { currentContext->framebuffers[n_semaphore].semaphore },
                signalSemaphores = { currentContext->framebuffers[c_semaphore].semaphore };
            std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

            auto smbi = vk::SubmitInfo()
                .setPWaitDstStageMask(waitStages.data()).setPWaitSemaphores(waitSemaphores.data()).setWaitSemaphoreCount(waitSemaphores.size())
                .setPCommandBuffers(&commandBuffer).setCommandBufferCount(1)
                .setPSignalSemaphores(signalSemaphores.data()).setSignalSemaphoreCount(signalSemaphores.size());

            // submit command once
            vte::submitCmd(currentContext->queue->device->logical, currentContext->queue->queue, { commandBuffer }, smbi);
            currentContext->queue->device->logical.freeCommandBuffers(currentContext->queue->commandPool, { commandBuffer });

            // reset wait semaphore
            //currentContext->queue->device->logical.destroySemaphore(currentContext->framebuffers[n_semaphore].semaphore);
            //currentContext->framebuffers[n_semaphore].semaphore = currentContext->queue->device->logical.createSemaphore(vk::SemaphoreCreateInfo());
        }

        // present for displaying of this image
        currentContext->queue->queue.presentKHR(vk::PresentInfoKHR(
            1, &currentContext->framebuffers[c_semaphore].semaphore,
            1, &currentContext->swapchain,
            &currentBuffer, nullptr
        ));
    }

    glfwDestroyWindow(window); glfwTerminate();
    //system("pause");


}
