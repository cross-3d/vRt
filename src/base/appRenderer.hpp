#pragma once
#include "appBase.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>

namespace rnd {

    using namespace vrt;

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
        glm::mat4x4 camInv = glm::mat4x4(1.f), projInv = glm::mat4x4(1.f);
        glm::vec4 sceneRes = glm::vec4(1.f);
        int enable360 = 0, variant = 0, r1 = 0, r2 = 0;
    };

    struct VtBvhUniformDebug {
        glm::mat4x4 transform = glm::mat4x4(1.f), transformInv = glm::mat4x4(1.f);
        glm::mat4x4 projection = glm::mat4x4(1.f), projectionInv = glm::mat4x4(1.f);
        int leafCount = 0, primitiveCount = 0, r1 = 0, r2 = 0;
    };

    struct VtLeafDebug {
        glm::vec4 boxMn;
        glm::vec4 boxMx;
        glm::ivec4 pdata;
    };


    template<class T>
    inline auto writeIntoBuffer(vte::Queue deviceQueue, const std::vector<T>& vctr, const vrt::VtDeviceBuffer& dBuffer, VkDeviceSize byteOffset = 0) {
        VkResult result = VK_SUCCESS;
        vrt::vtSetBufferSubData<T>(vctr, deviceQueue->device->rtDev);
        vte::submitOnce(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](VkCommandBuffer cmdBuf) {
            VkBufferCopy bfc = { 0, byteOffset, vte::strided<T>(vctr.size()) };
            vrt::vtCmdCopyHostToDeviceBuffer(cmdBuf, deviceQueue->device->rtDev, dBuffer, 1, &bfc);
        });
        return result;
    };

    template<class T>
    inline auto writeIntoImage(vte::Queue deviceQueue, const std::vector<T>& vctr, const vrt::VtDeviceImage& dImage, VkDeviceSize byteOffset = 0) {
        VkResult result = VK_SUCCESS;
        vrt::vtSetBufferSubData<T>(vctr, deviceQueue->device->rtDev);
        vte::submitOnce(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](VkCommandBuffer cmdBuf) {
            VkBufferImageCopy bfc = { 0, dImage->_extent.width, dImage->_extent.height, dImage->_subresourceLayers, VkOffset3D{0u,0u,0u}, dImage->_extent };
            vrt::vtCmdImageBarrier(cmdBuf, dImage);
            vrt::vtCmdCopyHostToDeviceImage(cmdBuf, deviceQueue->device->rtDev, dImage, 1, &bfc);
        });
        return result;
    };

    template<class T>
    inline auto readFromBuffer(vte::Queue deviceQueue, const vrt::VtDeviceBuffer& dBuffer, std::vector<T>& vctr, VkDeviceSize byteOffset = 0) {
        VkResult result = VK_SUCCESS;
        vte::submitOnce(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](VkCommandBuffer cmdBuf) {
            VkBufferCopy bfc = { byteOffset, 0, vte::strided<T>(vctr.size()) };
            vrt::vtCmdCopyDeviceBufferToHost(cmdBuf, dBuffer, deviceQueue->device->rtDev, 1, &bfc);
        });
        vrt::vtGetBufferSubData<T>(deviceQueue->device->rtDev, vctr);
        return result;
    };


/*  // no support by GCC compilers
    template<class T>
    inline auto readFromBuffer(vte::Queue deviceQueue, const std::shared_ptr<_vt::BufferRegion>& dBuffer, std::vector<T>& vctr, VkDeviceSize byteOffset = 0) {
        VkResult result = VK_SUCCESS;
        vte::submitOnce(deviceQueue->device->rtDev, deviceQueue->queue, deviceQueue->commandPool, [&](VkCommandBuffer cmdBuf) {
            VkBufferCopy bfc = { dBuffer->_offset + byteOffset, 0ull, vte::strided<T>(vctr.size()) };
            vrt::vtCmdCopyDeviceBufferToHost(cmdBuf, vrt::VtDeviceBuffer{ dBuffer->_boundBuffer }, deviceQueue->device->rtDev, 1, &bfc);
        });
        vrt::vtGetBufferSubData<T>(deviceQueue->device->rtDev, vctr);
        return result;
    };
    */

    inline auto createBufferFast(vte::Queue deviceQueue, vrt::VtDeviceBuffer& dBuffer, VkDeviceSize byteSize = 1024 * 16) {
        vrt::VtDeviceBufferCreateInfo dbs;
        dbs.usageFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        dbs.bufferSize = byteSize;
        dbs.familyIndex = deviceQueue->familyIndex;
        dbs.format = VK_FORMAT_R16G16_UINT;
        vrt::vtCreateDeviceBuffer(deviceQueue->device->rtDev, &dbs, &dBuffer);
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
    };

    inline auto _getFormat(const tinygltf::Accessor& accs) {
        vrt::VtFormatDecomp format = {};

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

        uint32_t compType = vrt::VT_TYPE_FLOAT;
        if (accs.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || accs.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) { compType = vrt::VT_TYPE_UINT16; };
        if (accs.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT || accs.componentType == TINYGLTF_COMPONENT_TYPE_INT) { compType = vrt::VT_TYPE_UINT32; };
        // TODO float16 support

        format.setComponents(size);
        format.setType(compType);
        format.setNormalized(accs.normalized);

        return format;
    };




    struct Active {
        std::vector<uint8_t> keys = {};
        std::vector<uint8_t> mouse = {};

        double mX = 1e-5, mY = 1e-5, dX = 0.0, dY = 0.0;
        double tDiff = 0.0, tCurrent = 1e-5;
    };
    
    class Renderer;


    class Shared : public std::enable_shared_from_this<Shared> {
        public: 
        static Active active; // shared properties
        static GLFWwindow* window; // in-bound GLFW window
        friend Renderer;
        
        static void TimeCallback(double milliseconds = 1e-5) {
            Shared::active.tDiff = milliseconds - Shared::active.tCurrent, Shared::active.tCurrent = milliseconds;
        };

        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS) Shared::active.keys[key] = uint8_t(1u);
            if (action == GLFW_RELEASE) Shared::active.keys[key] = uint8_t(0u);
            if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) { glfwTerminate(); exit(0); };
        };

        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
            if (action == GLFW_PRESS) Shared::active.mouse[button] = uint8_t(1u);
            if (action == GLFW_RELEASE) Shared::active.mouse[button] = uint8_t(0u);
        };

        static void MouseMoveCallback(GLFWwindow* window, double xpos = 1e-5, double ypos = 1e-5) {
            Shared::active.dX = xpos - Shared::active.mX, Shared::active.dY = ypos - Shared::active.mY; // get diff with previous position
            Shared::active.mX = xpos, Shared::active.mY = ypos; // set current mouse position
        };
    };


    // camera binding control 
    class CameraController : public std::enable_shared_from_this<CameraController> {
    public:
        // use pointers
        glm::vec3* viewVector = nullptr;
        glm::vec3* eyePos = nullptr;
        glm::vec3* upVector = nullptr;
        glm::uvec2* canvasSize = nullptr;

        // create relative control matrice
        auto project() { return glm::lookAt(*eyePos, (*eyePos + *viewVector), *upVector); };

        // event handler
        void handle() {
            //auto mPtr = (glm::dvec2*)&Shared::active.mX, mDiff = *mPtr - mousePosition;
            auto mDiff = glm::dvec2(Shared::active.dX, Shared::active.dY);
            auto &diff = Shared::active.tDiff;

            glm::mat4 viewm = this->project();
            glm::mat4 unviewm = glm::inverse(viewm);
            glm::vec3 ca = (viewm * glm::vec4(*eyePos, 1.0f)).xyz(), vi = glm::normalize((glm::vec4(*viewVector, 0.0) * unviewm).xyz());
            bool isFocus = true;

            if (Shared::active.mouse.size() > 0 && Shared::active.mouse[GLFW_MOUSE_BUTTON_LEFT] && isFocus)
            {
                if (glm::abs(mDiff.x) > 0.0) this->rotateX(vi, mDiff.x);
                if (glm::abs(mDiff.y) > 0.0) this->rotateY(vi, mDiff.y);
            }

            if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_W] && isFocus)
            {
                this->forwardBackward(ca, -diff);
            }

            if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_S] && isFocus)
            {
                this->forwardBackward(ca, diff);
            }

            if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_A] && isFocus)
            {
                this->leftRight(ca, -diff);
            }

            if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_D] && isFocus)
            {
                this->leftRight(ca, diff);
            }

            if (Shared::active.keys.size() > 0 && (Shared::active.keys[GLFW_KEY_SPACE] || Shared::active.keys[GLFW_KEY_E]) && isFocus)
            {
                this->topBottom(ca, diff);
            }

            if (Shared::active.keys.size() > 0 && (Shared::active.keys[GLFW_KEY_LEFT_SHIFT] || Shared::active.keys[GLFW_KEY_C] || Shared::active.keys[GLFW_KEY_Q]) && isFocus)
            {
                this->topBottom(ca, -diff);
            }

            *viewVector = glm::normalize((glm::vec4(vi, 0.0) * viewm).xyz());
            *eyePos = (unviewm * glm::vec4(ca, 1.0f)).xyz();
        }


        // sub-contollers
        void leftRight(glm::vec3 &ca, const float &diff) {
            ca.x += diff / 100.0f;
        }
        
        void topBottom(glm::vec3 &ca, const float &diff) {
            ca.y += diff / 100.0f;
        }

        void forwardBackward(glm::vec3 &ca, const float &diff) {
            ca.z += diff / 100.0f;
        }

        void rotateY(glm::vec3 &vi, const float &diff) {
            glm::mat4 rot = glm::rotate(diff / float(canvasSize->y) * 2.f, glm::vec3(-1.0f, 0.0f, 0.0f));
            vi = (rot * glm::vec4(vi, 1.0f)).xyz();
        }

        void rotateX(glm::vec3 &vi, const float &diff) {
            glm::mat4 rot = glm::rotate(diff / float(canvasSize->x) * 2.f, glm::vec3(0.0f, -1.0f, 0.0f));
            vi = (rot * glm::vec4(vi, 1.0f)).xyz();
        }
    };


    // defined inline static members of shared area
    inline Active Shared::active = Active{};
    inline GLFWwindow* Shared::window = nullptr;

    // renderer biggest class 
    class Renderer : public std::enable_shared_from_this<Renderer> {
    public: 
        GLFWwindow* window = nullptr; // bound GLFW window

        std::shared_ptr<CameraController> cameraController = nullptr;
        std::shared_ptr<vte::ApplicationBase> appBase = nullptr;
        std::shared_ptr<vte::GraphicsContext> currentContext = nullptr;
        vk::Instance instance = {};
        vk::PhysicalDevice gpu = {};
        vk::RenderPass renderpass = {};
        vte::Queue deviceQueue = {};
        
        float guiScale = 1.0f;
        uint32_t canvasWidth = 1, canvasHeight = 1; // canvas size with SSAA accounting
        uint32_t windowWidth = 1, windowHeight = 1; // virtual window size (without DPI recalculation)
        uint32_t realWidth = 1, realHeight = 1; // real window size (with DPI accounting)
        uint32_t gpuID = 1;

        // current rendering state
        int32_t currSemaphore = -1; uint32_t currentBuffer = 0;

        // names and directories
        std::string title = "vRt pre-alpha";
        std::string shaderPrefix = "./shaders/", shaderPack = "./shaders/";
        std::string bgTexName = "";
        std::string modelInput = "";
        std::string directory = "./";
        
        // timings
        double modelScale = 1.0, tPastFramerateStreamF = 60.0, tPast = 1e-5; int32_t reflectionLevel = 0, transparencyLevel = 0;
        std::chrono::high_resolution_clock::time_point tStart; // starting time

        // camera position 
        const glm::vec3 scale = glm::vec3(1.f); // correction for precisions
        glm::vec3 eyePos = glm::vec3(-3.5f, 10.5f, -50.6f).zyx();
        glm::vec3 viewVector = glm::vec3(1e-4f, 1e-4f, 1.f).zyx();
        glm::vec3 moveVector = glm::vec3(1e-4f, 1e-4f, 1.f).zyx();
        glm::vec3 upVector = glm::vec3(1e-5f, 1.f, 1e-5f);

        // precision correction for boxes
        glm::vec4 optDensity = glm::vec4(glm::vec3(1.f) / glm::vec3(1.f), 1.f);

        // output objects
        VtDeviceImage outputImage = {}, normalPass = {}, originPass = {}, specularPass = {}, depthImage = {};
        vk::Framebuffer firstGenFramebuffer = {};
        vk::RenderPass firstGenRenderpass = {};
        vk::Pipeline firstGenPipeline = {};


        // ray tracing objects
        std::vector<VtDeviceBuffer> VDataSpace;
        std::vector<VkBufferView> bviews;
        std::vector<VtVertexAccessor> accessors = {};
        std::vector<VtVertexBufferView> bufferViews = {};
        std::vector<std::vector<VtVertexInputSet>> vertexInputs;
        std::vector<VtVertexInputSet> inputs;
        std::vector<glm::mat4> transforms;
        std::vector<vk::DescriptorSetLayout> customedLayouts;
        std::vector<vk::DescriptorSet> drawDescriptorSets;

        VtDeviceImage envImage = {};
        VkSampler dullSampler = {};
        std::vector<VtDeviceImage> mImages;
        std::vector<VkSampler> mSamplers;

        VtDeviceBuffer materialDescs;
        VtDeviceBuffer materialCombImages;
        VkDescriptorSet usrDescSet, vtxDescSet;
        VtMaterialSet materialSet;
        VtRayTracingSet raytracingSet;
        VtPipelineLayout rtPipelineLayout, rtVPipelineLayout;
        VtPipeline rtPipeline, rfPipeline;
        VtVertexAssemblyPipeline vtxPipeline;
        VtAcceleratorSet accelerator;
        VtVertexAssemblySet vertexAssembly;
        VkCommandBuffer bCmdBuf, rtCmdBuf, vxCmdBuf, vxuCmdBuf;
        VtDeviceBuffer rtUniformBuffer;
        VtCameraUniform cameraUniformData;

        // vertex input buffer objects
        VtDeviceBuffer VBufferRegions, VBufferView, VAccessorSet, VAttributes, VTransforms;
        tinygltf::Model model = {}; tinygltf::TinyGLTF loader = {};

        // attribute bindings
        const uint32_t NORMAL_TID = 0;
        const uint32_t TEXCOORD_TID = 1;
        const uint32_t TANGENT_TID = 2;
        const uint32_t BITANGENT_TID = 3;
        const uint32_t VCOLOR_TID = 4;

        // methods for rendering
        void Arguments(int argc = 0, char** argv = nullptr);
        void Init(uint32_t canvasWidth, uint32_t canvasHeight);
        void InitRayTracing();
        void InitPipeline();
        void InitCommands();
        void Preload(const std::string& modelName = "");
        void Precompute();
        void ComputeRayTracing();
        void Draw();
        void InitRayTracingPipeline();
        void HandleData();
        void CreateFbo();

    };

};
