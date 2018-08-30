#pragma once
#include "gltfPBR.inl"

// initial renderer
const uint32_t canvasWidth = 640, canvasHeight = 360; // without accounting super-sampling
std::shared_ptr<rnd::Renderer> renderer = nullptr;

// main program with handling
int main(int argc, char** argv) {

    // initialize API
    if (!glfwInit() || !glfwVulkanSupported()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // initialize renderer
    renderer = std::make_shared<rnd::Renderer>();
    renderer->Arguments(argc, argv);
    renderer->Init(canvasWidth, canvasHeight); // init GLFW window there
    renderer->InitPipeline();
    renderer->InitRayTracing();
    renderer->InitRayTracingPipeline();
    renderer->Preload();
    renderer->InitCommands();
    
    // looping rendering
    while (!glfwWindowShouldClose(renderer->window)) {
        glfwPollEvents();
        renderer->Precompute();
        renderer->ComputeRayTracing();
        renderer->Draw();
        renderer->HandleData();
    };
    
    glfwDestroyWindow(renderer->window); 
    glfwTerminate();
    return 0;
}
