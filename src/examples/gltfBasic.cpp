
#include <appBase.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>


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
    int enable360 = 0, variant = 0, r1 = 0, r2 = 0;
};

struct VtBvhUniformDebug {
    glm::mat4x4 transform = glm::mat4x4(1.f);
    glm::mat4x4 transformInv = glm::mat4x4(1.f);
    glm::mat4x4 projection = glm::mat4x4(1.f);
    glm::mat4x4 projectionInv = glm::mat4x4(1.f);
    int leafCount = 0, primitiveCount = 0, r1 = 0, r2 = 0;
};


struct VtLeafDebug {
    glm::vec4 boxMn;
    glm::vec4 boxMx;
    glm::ivec4 pdata;
};


// application fast utility for fill buffers
template<class T>
inline auto writeIntoBuffer(vte::Queue deviceQueue, const std::vector<T>& vctr, const vt::VtDeviceBuffer& dBuffer, size_t byteOffset = 0) {
    VkResult result = VK_SUCCESS;
    vt::vtSetBufferSubData<T>(vctr, deviceQueue->device->rtDev);
    vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
        vt::vtCmdCopyHostToDeviceBuffer(cmdBuf, deviceQueue->device->rtDev, dBuffer, 1, (const VkBufferCopy *)&(vk::BufferCopy(0, byteOffset, vte::strided<T>(vctr.size()))));
    });
    return result;
};

template<class T>
inline auto writeIntoImage(vte::Queue deviceQueue, const std::vector<T>& vctr, const vt::VtDeviceImage& dImage, size_t byteOffset = 0) {
    VkResult result = VK_SUCCESS;
    vt::vtSetBufferSubData<T>(vctr, deviceQueue->device->rtDev);
    vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
        //vt::vtCmdCopyHostToDeviceBuffer(cmdBuf, deviceQueue->device->rtDev, dImage, 1, (const VkBufferCopy *)&(vk::BufferCopy(0, byteOffset, vte::strided<T>(vctr.size()))));
        vt::vtCmdCopyHostToDeviceImage(cmdBuf, deviceQueue->device->rtDev, dImage, 1, &(VkBufferImageCopy{ 0, dImage->_extent.width, dImage->_extent.height, dImage->_subresourceRange, {0u,0u,0u}, dImage->_extent }));
    });
    return result;
};

template<class T>
inline auto readFromBuffer(vte::Queue deviceQueue, const vt::VtDeviceBuffer& dBuffer, std::vector<T>& vctr, size_t byteOffset = 0) {
    VkResult result = VK_SUCCESS;
    vte::submitOnce(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
        vt::vtCmdCopyDeviceBufferToHost(cmdBuf, dBuffer, deviceQueue->device->rtDev, 1, (const VkBufferCopy *)&(vk::BufferCopy(0, byteOffset, vte::strided<T>(vctr.size()))));
    });
    vt::vtGetBufferSubData<T>(deviceQueue->device->rtDev, vctr);
    return result;
};


inline auto createBufferFast(vte::Queue deviceQueue, vt::VtDeviceBuffer& dBuffer, size_t byteSize = 1024 * 16) {
    vt::VtDeviceBufferCreateInfo dbs;
    dbs.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    dbs.bufferSize = byteSize;
    dbs.familyIndex = deviceQueue->familyIndex;
    dbs.format = VK_FORMAT_R16G16_UINT;
    vt::vtCreateDeviceBuffer(deviceQueue->device->rtDev, &dbs, &dBuffer);
};

inline auto getShaderDir(const uint32_t& vendorID) {
    std::string shaderDir = "./universal/";
    switch (vendorID) {
        case 4318:
            shaderDir = "./nvidia/";
            break;
        case 4098:
            shaderDir = "./amd/";
            break;
        case 8086: // x86 ID, WHAT?
            shaderDir = "./intel/";
            break;
    }
    return shaderDir;
}


inline auto _getFormat(const tinygltf::Accessor& accs) {
    vt::VtFormatDecomp format = {};

    uint32_t size = 1;
    if (accs.type == TINYGLTF_TYPE_SCALAR) {
        size = 1;
    } else if (accs.type == TINYGLTF_TYPE_VEC2) {
        size = 2;
    } else if (accs.type == TINYGLTF_TYPE_VEC3) {
        size = 3;
    } else if (accs.type == TINYGLTF_TYPE_VEC4) {
        size = 4;
    } else {
        assert(0);
    }

    uint32_t compType = vt::VT_TYPE_FLOAT;
    if (accs.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || accs.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) { compType = vt::VT_TYPE_UINT16; };
    if (accs.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT || accs.componentType == TINYGLTF_COMPONENT_TYPE_INT) { compType = vt::VT_TYPE_UINT32; };
    // TODO float16 support

    format.setComponents(size);
    format.setType(compType);
    format.setNormalized(accs.normalized);

    return format;
}


void main() {
    using namespace vt;

    if (!glfwInit()) exit(EXIT_FAILURE);
    if (!glfwVulkanSupported()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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
    std::cout << devProperties.vendorID << std::endl;

    // create combined device object
    const std::string shaderPack = "./shaders/" + getShaderDir(devProperties.vendorID);
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
    VtDeviceImage outputImage;
    vtCreateDeviceImage(deviceQueue->device->rtDev, &dii, &outputImage);

    // dispatch image barrier
    vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
        vtCmdImageBarrier(cmdBuf, outputImage);
    });



    tinygltf::Model model = {};
    tinygltf::TinyGLTF loader = {};
    std::string err, input_filename = 
        //"models/vokselia_spawn.gltf";
        "models/sponza/sponza.gltf";
        //"models/BoomBoxWithAxes.gltf";
        //"models/Chess_Set.gltf";
        //"models/Cube.gltf";
        //"models/scene.gltf";
        //"models/BoomBox.gltf";

    bool ret = loader.LoadASCIIFromFile(&model, &err, input_filename.c_str());


    //model
    std::vector<VtDeviceBuffer> VDataSpace;
    for (auto& b : model.buffers) {
        VtDeviceBuffer buf;
        createBufferFast(deviceQueue, buf, b.data.size());
        if (b.data.size() > 0) writeIntoBuffer(deviceQueue, b.data, buf);
        VDataSpace.push_back(buf);
    }
    std::vector<VkBufferView> bviews; for (auto&b : VDataSpace) { bviews.push_back(b); };





    //////////////////////////////////////
    // ray tracing preparing long stage //
    //////////////////////////////////////

    VtDeviceImage envImage;
    VtDeviceImage dullImage;
    VkSampler dullSampler;
    VtDeviceBuffer materialDescs;
    VtDeviceBuffer materialCombImages;
    VkDescriptorSet usrDescSet, vtxDescSet;
    std::vector<vk::DescriptorSetLayout> customedLayouts;
    VtMaterialSet materialSet;
    VtRayTracingSet raytracingSet;
    VtPipelineLayout rtPipelineLayout, rtVPipelineLayout;
    VtPipeline rtPipeline;
    VtVertexAssemblyPipeline vtxPipeline;
    VtAcceleratorSet accelerator;
    VtVertexAssemblySet vertexAssembly;
    //VtVertexInputSet vertexInput, vertexInput2; // arrayed
    VkCommandBuffer bCmdBuf, rtCmdBuf, vxCmdBuf, vxuCmdBuf;
    VtDeviceBuffer rtUniformBuffer;

    // mesh list
    std::vector<std::vector<VtVertexInputSet>> vertexInputs;

    // create vertex input buffer objects
    VtDeviceBuffer VBufferRegions, VBufferView, VAccessorSet, VAttributes, VTransforms;
    {
        createBufferFast(deviceQueue, VBufferRegions, sizeof(VtVertexRegionBinding));
        createBufferFast(deviceQueue, VAccessorSet, sizeof(VtVertexAccessor) * model.accessors.size());
        createBufferFast(deviceQueue, VBufferView, sizeof(VtVertexBufferView) * model.bufferViews.size());
        createBufferFast(deviceQueue, VAttributes, sizeof(VtVertexAttributeBinding) * 1024 * 1024);
        createBufferFast(deviceQueue, VTransforms, sizeof(glm::mat4) * 1024 * 1024);
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
        VtAppMaterial redEmission, greenEmission;
        greenEmission.emissive = glm::vec4(0.1f, 0.5f, 0.1f, 1.f);
        redEmission.emissive = glm::vec4(0.5f, 0.1f, 0.1f, 1.f);
        writeIntoBuffer<VtAppMaterial>(deviceQueue, { greenEmission, redEmission }, materialDescs, 0);
    }


    {
        // create dull material description set
        createBufferFast(deviceQueue, materialCombImages, vte::strided<VtVirtualCombinedImage>(1));

        // set first buffer data
        VtVirtualCombinedImage initialMaterialDesc;
        writeIntoBuffer<VtVirtualCombinedImage>(deviceQueue, { initialMaterialDesc }, materialCombImages,  0);
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


    glm::vec3 eyePos = glm::vec3(0.f, 10.5f, -40.6f).zyx();
    //glm::vec3 eyePos = glm::vec3(0.f, 1.0f, 10.6f);
    glm::vec3 viewVector = glm::vec3(0.f, 0.f, 1.f).zyx();
    glm::vec3 moveVector = glm::vec3(0.f, 0.f, 1.f).zyx();

    // initial matrices
    auto scale = 10.0f * glm::vec3(1.f, 1.f, 1.f);
    auto atMatrix = glm::lookAt(eyePos*scale, (eyePos+viewVector)*scale, glm::vec3(0.f, 1.f, 0.f));
    auto pjMatrix = glm::perspective(float(M_PI) / 3.f, 16.f / 9.f, 0.0001f, 1000.f);

    // set first uniform buffer data
    VtCameraUniform cameraUniformData;
    cameraUniformData.projInv = glm::transpose(glm::inverse(pjMatrix));
    cameraUniformData.camInv = glm::transpose(glm::inverse(atMatrix));
    cameraUniformData.sceneRes = glm::vec4(1280.f, 720.f, 1.f, 1.f);
    cameraUniformData.variant = 1;

    {
        // create uniform buffer
        createBufferFast(deviceQueue, rtUniformBuffer, vte::strided<VtCameraUniform>(1));
        //writeIntoBuffer<VtCameraUniform>(deviceQueue, { cameraUniformData }, rtUniformBuffer, 0);
    }



    {
        // custom bindings for ray tracing systems
        const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // constants for generation shader
            vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // env map for miss shader
            vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute) // env map for miss shader
        };
        customedLayouts.push_back(deviceQueue->device->logical.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size())));

        // create descriptor set, based on layout
        // TODO: fill descriptor set by inputs
        auto dsc = deviceQueue->device->logical.allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(deviceQueue->device->descriptorPool).setPSetLayouts(customedLayouts.data()).setDescriptorSetCount(customedLayouts.size()));
        usrDescSet = dsc[0];
    }

    { 
        // write ray tracing user defined descriptor set
        auto _write_tmpl = vk::WriteDescriptorSet(usrDescSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo(&vk::DescriptorBufferInfo(rtUniformBuffer->_descriptorInfo())),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setPImageInfo(&vk::DescriptorImageInfo(envImage->_descriptorInfo()).setSampler(dullSampler)),
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(2).setDescriptorType(vk::DescriptorType::eStorageImage).setPImageInfo(&vk::DescriptorImageInfo(outputImage->_descriptorInfo())),
        };
        vk::Device(deviceQueue->device->rtDev).updateDescriptorSets(writes, {});
    }

    {
        // create pipeline layout for ray tracing
        vk::PipelineLayoutCreateInfo vpi;
        vpi.pSetLayouts = customedLayouts.data();
        vpi.setLayoutCount = customedLayouts.size();

        VtPipelineLayoutCreateInfo vpti;
        vpti.pGeneralPipelineLayout = &(VkPipelineLayoutCreateInfo)vpi;

        vtCreateRayTracingPipelineLayout(deviceQueue->device->rtDev, &vpti, &rtPipelineLayout);
    }



    {
        // custom bindings for ray tracing systems
        const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute),
        };
        customedLayouts.push_back(deviceQueue->device->logical.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo().setPBindings(_bindings.data()).setBindingCount(_bindings.size())));

        // create descriptor set, based on layout
        // TODO: fill descriptor set by inputs
        auto dsc = deviceQueue->device->logical.allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(deviceQueue->device->descriptorPool).setPSetLayouts(customedLayouts.data()).setDescriptorSetCount(customedLayouts.size()));
        vtxDescSet = dsc[0];
    }

    {
        // write ray tracing user defined descriptor set
        auto _write_tmpl = vk::WriteDescriptorSet(vtxDescSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(_write_tmpl).setDstBinding(0).setDescriptorType(vk::DescriptorType::eStorageBuffer).setPBufferInfo(&vk::DescriptorBufferInfo(VTransforms->_descriptorInfo())),
        };
        vk::Device(deviceQueue->device->rtDev).updateDescriptorSets(writes, {});
    }

    {
        // create pipeline layout for ray tracing
        vk::PipelineLayoutCreateInfo vpi;
        vpi.pSetLayouts = customedLayouts.data();
        vpi.setLayoutCount = customedLayouts.size();

        VtPipelineLayoutCreateInfo vpti;
        vpti.pGeneralPipelineLayout = &(VkPipelineLayoutCreateInfo)vpi;

        vtCreateVertexAssemblyPipelineLayout(deviceQueue->device->rtDev, &vpti, &rtVPipelineLayout);
    }

    {
        // create ray tracing pipeline
        VtVertexAssemblyPipelineCreateInfo vtpi;
        vtpi.vertexAssemblyModule = vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "vertex/vtransformed.comp.spv"));
        vtpi.pipelineLayout = rtVPipelineLayout;
        vtCreateVertexAssemblyPipeline(deviceQueue->device->rtDev, &vtpi, &vtxPipeline);
    }







    {
        // create ray tracing set
        VtRayTracingSetCreateInfo rtsi;
        rtsi.maxRays = 1280 * 720; // prefer that limit
        vtCreateRayTracingSet(deviceQueue->device->rtDev, &rtsi, &raytracingSet);
    }

    {
        // create material set
        VtMaterialSetCreateInfo mtsi;
        mtsi.imageCount = 1; mtsi.pImages = &dullImage->_descriptorInfo();
        mtsi.samplerCount = 1; mtsi.pSamplers = (VkSampler *)&dullSampler;
        mtsi.bMaterialDescriptionsBuffer = materialDescs;
        mtsi.bImageSamplerCombinations = materialCombImages;
        mtsi.materialCount = 2;
        vtCreateMaterialSet(deviceQueue->device->rtDev, &mtsi, &materialSet);
    }

    {
        // create ray tracing pipeline
        VtRayTracingPipelineCreateInfo rtpi;
        rtpi.pGenerationModule = &vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/generation-shader.comp.spv"));
        rtpi.pClosestModules = &vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/closest-hit-shader.comp.spv"));
        rtpi.pMissModules = &vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/miss-hit-shader.comp.spv"));
        rtpi.pGroupModules = &vte::loadAndCreateShaderModuleStage(deviceQueue->device->rtDev, vte::readBinary(shaderPack + "rayTracing/group-shader.comp.spv"));

        rtpi.closestModuleCount = 1;
        rtpi.missModuleCount = 1;
        rtpi.groupModuleCount = 1;

        rtpi.pipelineLayout = rtPipelineLayout;
        vtCreateRayTracingPipeline(deviceQueue->device->rtDev, &rtpi, &rtPipeline);
    }


    // create accelerator set
    VtAcceleratorSetCreateInfo acci;
    acci.maxPrimitives = 1024 * 1024 * 4;
    acci.entryID = 0;
    vtCreateAccelerator(deviceQueue->device->rtDev, &acci, &accelerator);


    // create vertex assembly
    VtVertexAssemblySetCreateInfo vtsi;
    vtsi.maxPrimitives = 1024 * 1024 * 4;
    vtCreateVertexAssembly(deviceQueue->device->rtDev, &vtsi, &vertexAssembly);



    std::vector<VtVertexAccessor> accessors = {};
    std::vector<VtVertexBufferView> bufferViews = {};
    for (auto &acs: model.accessors) { accessors.push_back(VtVertexAccessor{ uint32_t(acs.bufferView), uint32_t(acs.byteOffset), uint32_t(_getFormat(acs)) }); }
    for (auto &bv : model.bufferViews) { bufferViews.push_back(VtVertexBufferView{ uint32_t(bv.buffer), uint32_t(bv.byteOffset), uint32_t(bv.byteStride), uint32_t(bv.byteLength) }); }
    


    const uint32_t NORMAL_TID = 0;
    const uint32_t TEXCOORD_TID = 1;
    const uint32_t TANGENT_TID = 2;
    const uint32_t BITANGENT_TID = 3;
    const uint32_t VCOLOR_TID = 4;

    std::vector<VtVertexAttributeBinding> attributes = {};
    for (auto& msh: model.meshes) {
        std::vector<VtVertexInputSet> primitives;
        for (auto& prim : msh.primitives) {
            VtVertexInputSet primitive;

            uint32_t attribOffset = attributes.size();
            VtVertexInputCreateInfo vtii = {};
            vtii.verticeAccessor = 0;
            vtii.indiceAccessor = -1;
            vtii.topology = VT_TOPOLOGY_TYPE_TRIANGLES_LIST;


            for (auto& attr: prim.attributes) { //attr
                if (attr.first.compare("POSITION") == 0) {
                    vtii.verticeAccessor = attr.second;
                }

                if (attr.first.compare("NORMAL") == 0) {
                    attributes.push_back({ NORMAL_TID, uint32_t(attr.second) });
                }

                if (attr.first.compare("TEXCOORD_0") == 0) {
                    attributes.push_back({ TEXCOORD_TID, uint32_t(attr.second) });
                }
            }



            tinygltf::Accessor &idcAccessor = model.accessors[prim.indices];
            vtii.indiceAccessor = prim.indices;
            vtii.attributeCount = attributes.size() - attribOffset;
            vtii.primitiveCount = idcAccessor.count / 3;
            vtii.materialID = prim.material;
            vtii.pSourceBuffers = bviews.data();
            vtii.sourceBufferCount = bviews.size();
            vtii.bBufferAccessors = VAccessorSet;
            vtii.bBufferAttributeBindings = VAttributes;
            //vtii.attributeByteOffset = attribOffset * sizeof(VtVertexAttributeBinding);
            vtii.attributeOffset = attribOffset;
            vtii.bBufferRegionBindings = VBufferRegions;
            vtii.bBufferViews = VBufferView;
            vtii.vertexAssemblyPipeline = vtxPipeline;
            vtCreateVertexInputSet(deviceQueue->device->rtDev, &vtii, &primitive);

            primitives.push_back(primitive);
        }

        vertexInputs.push_back(primitives);
    }





    std::vector<VtVertexInputSet> inputs;
    std::vector<glm::mat4> transforms; // todo: support of transformations
    {
        std::shared_ptr<std::function<void(const tinygltf::Node &, glm::dmat4, int)>> vertexLoader = {};
        vertexLoader = std::make_shared<std::function<void(const tinygltf::Node &, glm::dmat4, int)>>([&](const tinygltf::Node & node, glm::dmat4 inTransform, int recursive)->void {
            glm::dmat4 localTransform(1.0);
            localTransform *= (node.matrix.size() >= 16 ? glm::make_mat4(node.matrix.data()) : glm::dmat4(1.0));
            localTransform *= (node.translation.size() >= 3 ? glm::translate(glm::make_vec3(node.translation.data())) : glm::dmat4(1.0));
            localTransform *= (node.scale.size() >= 3 ? glm::scale(glm::make_vec3(node.scale.data())) : glm::dmat4(1.0));
            localTransform *= (node.rotation.size() >= 4 ? glm::mat4_cast(glm::make_quat(node.rotation.data())) : glm::dmat4(1.0));

            glm::dmat4 transform = inTransform * localTransform;
            if (node.mesh >= 0) {
                auto& mesh = vertexInputs[node.mesh]; // load mesh object (it just vector of primitives)
                for (auto& geom : mesh) {
                    //geom->getUniformSet()->getStructure(p).setTransform(transform); // here is bottleneck with host-GPU exchange
                    inputs.push_back(geom);
                    transforms.push_back(glm::transpose(transform));
                    transforms.push_back(glm::inverse(transform));
                }
            }
            if (node.children.size() > 0 && node.mesh < 0) {
                for (int n = 0; n < node.children.size(); n++) {
                    if (recursive >= 0) (*vertexLoader)(model.nodes[node.children[n]], transform, recursive - 1);
                }
            }
        });

        // matrix with scaling
        double mscale = 1.0;
        glm::dmat4 matrix(1.0);
        matrix *= glm::scale(glm::dvec3(mscale, mscale, mscale));

        // load scene
        uint32_t sceneID = 0;
        if (model.scenes.size() > 0) {
            for (int n = 0; n < model.scenes[sceneID].nodes.size(); n++) {
                tinygltf::Node & node = model.nodes[model.scenes[sceneID].nodes[n]];
                (*vertexLoader)(node, glm::dmat4(matrix), 16);
            }
        }
    }

    
    writeIntoBuffer(deviceQueue, transforms, VTransforms, 0);
    writeIntoBuffer(deviceQueue, accessors, VAccessorSet, 0);
    writeIntoBuffer(deviceQueue, bufferViews, VBufferView, 0);
    writeIntoBuffer(deviceQueue, attributes, VAttributes, 0);



    // ray tracing command buffers creation

    {
        // make accelerator and vertex builder command
        vxCmdBuf = vte::createCommandBuffer(deviceQueue->device->rtDev, deviceQueue->commandPool, false, false);
        VtCommandBuffer qVxCmdBuf; vtQueryCommandInterface(deviceQueue->device->rtDev, vxCmdBuf, &qVxCmdBuf);
        vtCmdBindDescriptorSets(qVxCmdBuf, VT_PIPELINE_BIND_POINT_VERTEXASSEMBLY, rtVPipelineLayout, 0, 1, &vtxDescSet, 0, nullptr);
        vtCmdBindVertexAssembly(qVxCmdBuf, vertexAssembly);
        vtCmdBindVertexInputSets(qVxCmdBuf, inputs.size(), inputs.data());
        vtCmdBuildVertexAssembly(qVxCmdBuf);
        vkEndCommandBuffer(qVxCmdBuf);
    }

    {
        // make accelerator and vertex builder command
        vxuCmdBuf = vte::createCommandBuffer(deviceQueue->device->rtDev, deviceQueue->commandPool, false, false);
        VtCommandBuffer qVxuCmdBuf; vtQueryCommandInterface(deviceQueue->device->rtDev, vxuCmdBuf, &qVxuCmdBuf);
        vtCmdBindDescriptorSets(qVxuCmdBuf, VT_PIPELINE_BIND_POINT_VERTEXASSEMBLY, rtVPipelineLayout, 0, 1, &vtxDescSet, 0, nullptr);
        vtCmdBindVertexAssembly(qVxuCmdBuf, vertexAssembly);
        vtCmdBindVertexInputSets(qVxuCmdBuf, inputs.size(), inputs.data());
        vtCmdUpdateVertexAssembly(qVxuCmdBuf, 0, true);
        vkEndCommandBuffer(qVxuCmdBuf);
    }

    {
        // make accelerator and vertex builder command
        bCmdBuf = vte::createCommandBuffer(deviceQueue->device->rtDev, deviceQueue->commandPool, false, false);
        VtCommandBuffer qBCmdBuf; vtQueryCommandInterface(deviceQueue->device->rtDev, bCmdBuf, &qBCmdBuf);
        vtCmdBindAccelerator(qBCmdBuf, accelerator);
        vtCmdBindVertexAssembly(qBCmdBuf, vertexAssembly);
        vtCmdBindVertexInputSets(qBCmdBuf, inputs.size(), inputs.data());
        vtCmdBuildAccelerator(qBCmdBuf);
        vkEndCommandBuffer(qBCmdBuf);
    }

    {
        // make ray tracing command buffer
        rtCmdBuf = vte::createCommandBuffer(deviceQueue->device->rtDev, deviceQueue->commandPool, false, false);
        VtCommandBuffer qRtCmdBuf; vtQueryCommandInterface(deviceQueue->device->rtDev, rtCmdBuf, &qRtCmdBuf);
        vtCmdBindPipeline(qRtCmdBuf, VT_PIPELINE_BIND_POINT_RAYTRACING, rtPipeline);
        vtCmdBindMaterialSet(qRtCmdBuf, VtEntryUsageFlags(VT_ENTRY_USAGE_CLOSEST | VT_ENTRY_USAGE_MISS), materialSet);
        vtCmdBindDescriptorSets(qRtCmdBuf, VT_PIPELINE_BIND_POINT_RAYTRACING, rtPipelineLayout, 0, 1, &usrDescSet, 0, nullptr);
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
        auto& image = outputImage;

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



    // dispatch building vertex internal data
    vte::submitCmd(deviceQueue->device->rtDev, deviceQueue->queue, { vxCmdBuf });

    // dispatch building accelerators
    vte::submitCmd(deviceQueue->device->rtDev, deviceQueue->queue, { bCmdBuf });

    // dispatch ray tracing
    //vte::submitCmd(deviceQueue->device->rtDev, deviceQueue->queue, { rtCmdBuf });



    // rendering presentation 
    int32_t currSemaphore = -1; uint32_t currentBuffer = 0;
    auto tIdle = std::chrono::high_resolution_clock::now();
    auto tPast = std::chrono::high_resolution_clock::now();
    auto tPastFramerateStreamF = 60.0;
    auto tDiffF = 0.0;

    while (!glfwWindowShouldClose(appfw->window())) {
        glfwPollEvents();


        
        eyePos += float(tDiffF) * glm::normalize(moveVector)*0.001f;
        

        // spamming by update uniform value
        auto atMatrix = glm::lookAt(eyePos*scale, (eyePos + viewVector)*scale, glm::vec3(0.f, 1.f, 0.f));
        cameraUniformData.camInv = glm::transpose(glm::inverse(atMatrix));

        // update start position
        vte::submitOnceAsync(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](const VkCommandBuffer& cmdBuf) {
            vkCmdUpdateBuffer(cmdBuf, rtUniformBuffer, 0, sizeof(VtCameraUniform), &cameraUniformData);
            updateCommandBarrier(cmdBuf);
        });

        
        vte::submitCmd(deviceQueue->device->rtDev, deviceQueue->queue, { vxuCmdBuf });
        vte::submitCmd(deviceQueue->device->rtDev, deviceQueue->queue, { bCmdBuf });
        vte::submitCmd(deviceQueue->device->rtDev, deviceQueue->queue, { rtCmdBuf });
        //std::this_thread::sleep_for(std::chrono::milliseconds(1)); // unable in NVidia to barrrier


        /*{ // reserved field for computing code
            std::vector<uint32_t> debugCounters(2);
            readFromBuffer(deviceQueue, { vertexAssembly->_countersBuffer }, debugCounters);

            std::vector<VtBvhUniformDebug> debugUniform(1);
            readFromBuffer(deviceQueue, { accelerator->_bvhBlockUniform }, debugUniform);

            std::vector<uint64_t> debugMortons(vertexAssembly->_calculatedPrimitiveCount);
            readFromBuffer(deviceQueue, { accelerator->_mortonCodesBuffer }, debugMortons);

            std::vector<uint32_t> debugMortonIdc(vertexAssembly->_calculatedPrimitiveCount);
            readFromBuffer(deviceQueue, { accelerator->_mortonIndicesBuffer }, debugMortonIdc);

            std::vector<VtLeafDebug> debugLeafs(vertexAssembly->_calculatedPrimitiveCount);
            readFromBuffer(deviceQueue, { accelerator->_leafBuffer }, debugLeafs);

            std::vector<uint32_t> debugBvhCounters(8);
            readFromBuffer(deviceQueue, { accelerator->_countersBuffer }, debugBvhCounters);

            std::vector<glm::vec4> debugBvhGenBoxes(256);
            readFromBuffer(deviceQueue, { accelerator->_generalBoundaryResultBuffer }, debugBvhGenBoxes);

            std::vector<glm::vec4> debugBvhBoxes(vertexAssembly->_calculatedPrimitiveCount * 4);
            readFromBuffer(deviceQueue, { accelerator->_bvhBoxBuffer }, debugBvhBoxes);

            std::vector<glm::ivec4> debugBvhMeta(vertexAssembly->_calculatedPrimitiveCount * 2);
            readFromBuffer(deviceQueue, { accelerator->_bvhMetaBuffer }, debugBvhMeta);

            std::vector<uint32_t> debugLeafIdx(vertexAssembly->_calculatedPrimitiveCount);
            readFromBuffer(deviceQueue, { accelerator->_leafNodeIndices }, debugLeafIdx);

            std::vector<glm::vec4> debugBvhWorkBoxes(vertexAssembly->_calculatedPrimitiveCount * 4);
            readFromBuffer(deviceQueue, { accelerator->_onWorkBoxes }, debugBvhWorkBoxes);

            std::vector<uint32_t> debugFitFlags (vertexAssembly->_calculatedPrimitiveCount*2);
            readFromBuffer(deviceQueue, { accelerator->_fitStatusBuffer }, debugFitFlags);


            //std::vector<uint32_t> lhistogram(16 * 64);
            //std::vector<uint32_t> lprefixsum(16 * 64);
            //readFromBuffer(deviceQueue, { deviceQueue->device->rtDev->_radixSort->_histogramBuffer }, lhistogram);
            //readFromBuffer(deviceQueue, { deviceQueue->device->rtDev->_radixSort->_prefixSumBuffer }, lprefixsum);
        }*/
        

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


        auto tNow = std::chrono::high_resolution_clock::now();
        auto tDiff = tNow - tPast; tPast = tNow;

        std::stringstream tDiffStream;
        tDiffF = double(tDiff.count()) * 1e-6;
        tDiffStream << std::fixed << std::setprecision(2) << tDiffF;

        std::stringstream tFramerateStream;
        auto tFramerateStreamF = 1e9 / double(tDiff.count());
        tPastFramerateStreamF = tPastFramerateStreamF * 0.5 + tFramerateStreamF * 0.5;
        tFramerateStream << std::fixed << std::setprecision(0) << tPastFramerateStreamF;

        auto wTitle = "vRt : " + tDiffStream.str() + "ms / " + tFramerateStream.str() + "Hz";
        glfwSetWindowTitle(window, wTitle.c_str());

    }

    glfwDestroyWindow(window); glfwTerminate();
    //system("pause");


}
