#include <stdio.h>
#include <sstream>

#define RVT_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <appBase.hpp>

void main() {
    if (!glfwInit()) exit(EXIT_FAILURE);
    if (!glfwVulkanSupported()) exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);



    uint32_t gpuID = 0;
    uint32_t currentBuffer = 0;
    int32_t currSemaphore = -1;
    std::shared_ptr<vte::ApplicationBase> appfw = std::make_shared<vte::ApplicationBase>();
    
    std::string title = "vRt early test";
    uint32_t canvasWidth = 1280, canvasHeight = 720;
    float windowScale = 1.0;

    // create GLFW window
    GLFWwindow* window = glfwCreateWindow(canvasWidth, canvasHeight, "vRt early test", NULL, NULL);
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }


	// create vulkan instance
	auto instance = appfw->createInstance();

    // get physical devices
    auto physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.size() < 0) { glfwTerminate(); std::cerr << "Vulkan does not supported, or driver broken." << std::endl; exit(0); }

    // choice device
    if (gpuID >= physicalDevices.size()) { gpuID = physicalDevices.size() - 1; }
    if (gpuID < 0 || gpuID == -1) gpuID = 0;
    auto gpu = physicalDevices[gpuID];

    vk::PhysicalDeviceProperties devProperties = gpu.getProperties();
    std::cout << "Current Device: ";
    std::cout << devProperties.deviceName << std::endl;

    // create surface
    appfw->createWindowSurface(window, canvasWidth, canvasHeight, title);
    appfw->format(appfw->getSurfaceFormat(gpu));


	// shader pack
	const std::string shaderPack = "./shaders/amd/";

    // create basic Vulkan objects
    auto deviceQueue = appfw->createDeviceQueue(gpu, false, shaderPack); // create default graphical device
    auto renderpass = appfw->createRenderpass(deviceQueue);

    // resize buffers and canvas
    glfwGetWindowContentScale(window, &windowScale, nullptr);



    // create image output
    vt::VtDeviceImageCreateInfo dii;
    dii.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    dii.familyIndex = deviceQueue->familyIndex;
    dii.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
    dii.layout = VK_IMAGE_LAYOUT_GENERAL;
    dii.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    dii.size = {1280, 720, 1};

    // create device image
    vt::VtDeviceImage outImage;
    vt::vtCreateDeviceImage(deviceQueue->device->rtDev, &dii, &outImage);



    

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
        trianglePipeline = deviceQueue->device->logical.createGraphicsPipeline(deviceQueue->device->pipelineCache,
            vk::GraphicsPipelineCreateInfo()
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
			auto& commandBuffer = (currentContext->framebuffers[n_semaphore].commandBuffer = _vt::createCommandBuffer(currentContext->queue->device->logical, currentContext->queue->commandPool, false)); // do reference of cmd buffer
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
			_vt::submitCmd(currentContext->queue->device->logical, currentContext->queue->queue, { commandBuffer }, smbi);
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
