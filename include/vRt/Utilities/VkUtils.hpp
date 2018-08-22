#pragma once

// Include headers
#include "../vRt.h"

// Vulkan API for C++
#include <vulkan/vulkan.hpp>

// include STL
#include <fstream>
#include <future>
#include <iostream>
#include <stdexcept>
#include <array>
#include <random>
#include <algorithm>
#include <iterator>


#ifdef VRT_ENABLE_EXECUTION_POLICY
#include <execution>
#define VRT_ASYNC(F) std::async(std::launch::async|std::launch::deferred,F);
#else
#define VRT_ASYNC(F) std::async(F);
#endif


namespace _vt {
    constexpr const static inline auto DEFAULT_FENCE_TIMEOUT = 100000000000ll;
    constexpr const static inline auto INTENSIVITY = 1024ull;//4096ull;
    constexpr const static inline auto ATTRIB_EXTENT = 8ull; // no way to set more than it now
    constexpr const static inline auto VRT_USE_MORTON_32 = true;

    template <typename T>
    static inline auto sgn(T val) { return (T(0) < val) - (val < T(0)); }

    template<class T = uint32_t>
    static inline T tiled(T sz, T gmaxtile) {
        // return (int32_t)ceil((double)sz / (double)gmaxtile);
        return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
    }

    static inline double milliseconds() {
        auto duration = std::chrono::high_resolution_clock::now();
        double millis = std::chrono::duration_cast<std::chrono::nanoseconds>(
            duration.time_since_epoch())
            .count() /
            1000000.0;
        return millis;
    }

    template <class T>
    static inline size_t strided(size_t sizeo) { return sizeof(T) * sizeo; }

    // read binary (for SPIR-V)
    static inline auto readBinary(std::string filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        std::vector<uint32_t> data;
        if (file.is_open()) {
            std::streampos size = file.tellg();
            data.resize(tiled(size_t(size), sizeof(uint32_t)));
            file.seekg(0, std::ios::beg);
            file.read((char *)data.data(), size);
            file.close();
        } else {
            std::cerr << "Failure to open " + filePath << std::endl;
        }
        return data;
    };

    // read source (unused)
    static inline auto readSource(const std::string &filePath, const bool &lineDirective = false) {
        std::string content = "";
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open()) {
            std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl; return content;
        }
        std::string line = "";
        while (!fileStream.eof()) {
            std::getline(fileStream, line);
            if (lineDirective || line.find("#line") == std::string::npos)
                content.append(line + "\n");
        }
        fileStream.close();
        return content;
    };

    // general command buffer barrier
    static inline void commandBarrier(VkCommandBuffer cmdBuffer) {
        VkMemoryBarrier memoryBarrier = vk::MemoryBarrier{};
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            {}, //VK_DEPENDENCY_BY_REGION_BIT,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr);
    };

    // from host command buffer barrier
    static inline void fromHostCommandBarrier(VkCommandBuffer cmdBuffer) {
        VkMemoryBarrier memoryBarrier = vk::MemoryBarrier{};
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            {}, //VK_DEPENDENCY_BY_REGION_BIT,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr);
    };

    // to host command buffer barrier
    static inline void toHostCommandBarrier(VkCommandBuffer cmdBuffer) {
        VkMemoryBarrier memoryBarrier = vk::MemoryBarrier{};
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
            {}, //VK_DEPENDENCY_BY_REGION_BIT,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr);
    };

    static inline void updateCommandBarrier(VkCommandBuffer cmdBuffer) {
        commandBarrier(cmdBuffer);
    };

    // create secondary command buffers for batching compute invocations
    static inline auto createCommandBuffer(VkDevice device, VkCommandPool cmdPool, bool secondary = true, bool once = true) {
        VkCommandBuffer cmdBuffer = {};

        VkCommandBufferAllocateInfo cmdi = vk::CommandBufferAllocateInfo{};
        cmdi.commandPool = cmdPool;
        cmdi.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdi.commandBufferCount = 1;
        vkAllocateCommandBuffers(device, &cmdi, &cmdBuffer);

        VkCommandBufferInheritanceInfo inhi = vk::CommandBufferInheritanceInfo{};
        inhi.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

        VkCommandBufferBeginInfo bgi = vk::CommandBufferBeginInfo{};
        bgi.flags = {};
        bgi.flags = once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        bgi.pInheritanceInfo = secondary ? &inhi : nullptr;
        vkBeginCommandBuffer(cmdBuffer, &bgi);

        return cmdBuffer;
    };

    static inline auto makeShaderModuleInfo(const std::vector<uint32_t>& code) {
        VkShaderModuleCreateInfo smi = vk::ShaderModuleCreateInfo{};
        smi.pCode = (uint32_t *)code.data();
        smi.codeSize = code.size()*4;
        smi.flags = {};
        return smi;
    };

    // create shader module
    static inline auto createShaderModuleIntrusive(VkDevice device, const std::vector<uint32_t>& code, VkShaderModule& hndl) {
        auto shaderModuleInfo = makeShaderModuleInfo(code);
        vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &hndl); return hndl;
    };

    static inline auto createShaderModule(VkDevice device, const std::vector<uint32_t>& code) {
        VkShaderModule sm = vk::ShaderModule{}; return createShaderModuleIntrusive(device, code, sm); return sm;
    };

    // create shader module
    static inline auto makeComputePipelineStageInfo(VkDevice device, const std::vector<uint32_t>& code, const char * entry = "main") {
        VkPipelineShaderStageCreateInfo spi = vk::PipelineShaderStageCreateInfo{};
        spi.flags = {};
        createShaderModuleIntrusive(device, code, spi.module);
        spi.pName = entry;
        spi.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        spi.pSpecializationInfo = nullptr;
        return spi;
    };

    // create compute pipelines
    static inline auto createCompute(VkDevice device, const VkPipelineShaderStageCreateInfo& spi, VkPipelineLayout layout, VkPipelineCache cache) {
        VkComputePipelineCreateInfo cmpi = vk::ComputePipelineCreateInfo{};
        cmpi.flags = {};
        cmpi.layout = layout;
        cmpi.stage = spi;
        cmpi.basePipelineHandle = {};
        cmpi.basePipelineIndex = -1;

        VkPipeline pipeline = {};
        vkCreateComputePipelines(device, cache, 1, &cmpi, nullptr, &pipeline);
        return pipeline;
    };

    // create compute pipelines
    static inline auto createCompute(VkDevice device, const std::vector<uint32_t>& code, VkPipelineLayout layout, VkPipelineCache cache) {
        return createCompute(device, makeComputePipelineStageInfo(device, code), layout, cache);
    };

    // create compute pipelines
    static inline auto createCompute(VkDevice device, const std::string& path, VkPipelineLayout layout, VkPipelineCache cache) {
        return createCompute(device, readBinary(path), layout, cache);
    };





#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
    #define createComputeHC createComputeMemory
#else
    #define createComputeHC createCompute
#endif


    // add dispatch in command buffer (with default pipeline barrier)
    static inline VkResult cmdDispatch(VkCommandBuffer cmd, VkPipeline pipeline, uint32_t x = 1, uint32_t y = 1, uint32_t z = 1, bool barrier = true) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdDispatch(cmd, x, y, z);
        if (barrier) {
            commandBarrier(cmd); // put shader barrier
        }
        return VK_SUCCESS;
    };

    // low level copy command between (prefer for host and device)
    static inline VkResult cmdCopyBufferL(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<vk::BufferCopy>& regions, const std::function<void(VkCommandBuffer)>& barrierFn = commandBarrier) {
        vk::CommandBuffer(cmd).copyBuffer(srcBuffer, dstBuffer, regions);
        barrierFn(cmd); // put copy barrier
        return VK_SUCCESS;
    };


    // short data set with command buffer (alike push constant)
    template<class T>
    static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize offset, const std::vector<T>& data) {
        vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, data);
        //updateCommandBarrier(cmd);
        return VK_SUCCESS;
    };

    // short data set with command buffer (alike push constant)
    template<class T>
    static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize offset, const VkDeviceSize& size, const T*data) {
        vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, size, data);
        //updateCommandBarrier(cmd);
        return VK_SUCCESS;
    };


    // template function for fill buffer by constant value
    // use for create repeat variant
    template<uint32_t Rv>
    static inline VkResult cmdFillBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize size = VK_WHOLE_SIZE, intptr_t offset = 0) {
        vk::CommandBuffer(cmd).fillBuffer(vk::Buffer(dstBuffer), offset, size, Rv);
        //updateCommandBarrier(cmd);
        return VK_SUCCESS;
    };

    // make whole size buffer descriptor info
    static inline auto bufferDescriptorInfo(VkBuffer buffer, vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE) {
        return vk::DescriptorBufferInfo(buffer, offset, size);
    };

    // submit command (with async wait)
    static inline void submitCmd(VkDevice device, VkQueue queue, std::vector<VkCommandBuffer> cmds, vk::SubmitInfo smbi = {}) {
        // no commands 
        if (cmds.size() <= 0) return;

        smbi.commandBufferCount = cmds.size();
        smbi.pCommandBuffers = (vk::CommandBuffer *)cmds.data();

        VkFence fence = {}; VkFenceCreateInfo fin = vk::FenceCreateInfo{};
        vkCreateFence(device, &fin, nullptr, &fence);
        vkQueueSubmit(queue, 1, (const VkSubmitInfo *)&smbi, fence);
        vkWaitForFences(device, 1, &fence, true, DEFAULT_FENCE_TIMEOUT);
        vkDestroyFence(device, fence, nullptr);
    };

    // once submit command buffer
    static inline void submitOnce(VkDevice device, VkQueue queue, VkCommandPool cmdPool, std::function<void(VkCommandBuffer)> cmdFn = {}, vk::SubmitInfo smbi = {}) {
        auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); vkEndCommandBuffer(cmdBuf);
        submitCmd(device, queue, { cmdBuf });
        vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuf); // free that command buffer
    };

    // submit command (with async wait)
    static inline void submitCmdAsync(VkDevice device, VkQueue queue, std::vector<VkCommandBuffer> cmds, std::function<void()> asyncCallback = {}, vk::SubmitInfo smbi = {}) {
        // no commands 
        if (cmds.size() <= 0) return;

        smbi.commandBufferCount = cmds.size();
        smbi.pCommandBuffers = (const vk::CommandBuffer *)cmds.data();

        VkFence fence = {}; VkFenceCreateInfo fin = vk::FenceCreateInfo{};
        vkCreateFence(device, &fin, nullptr, &fence);
        vkQueueSubmit(queue, 1, (const VkSubmitInfo *)&smbi, fence);
        VRT_ASYNC([=]() {
            vkWaitForFences(device, 1, &fence, true, DEFAULT_FENCE_TIMEOUT);
            VRT_ASYNC([=]() {
                vkDestroyFence(device, fence, nullptr);
                if (asyncCallback) asyncCallback();
            });
        });
    };

    // once submit command buffer
    static inline void submitOnceAsync(VkDevice device, VkQueue queue, VkCommandPool cmdPool, std::function<void(VkCommandBuffer)> cmdFn = {}, std::function<void(VkCommandBuffer)> asyncCallback = {}, vk::SubmitInfo smbi = {}) {
        auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); vkEndCommandBuffer(cmdBuf);
        submitCmdAsync(device, queue, { cmdBuf }, [=]() {
            asyncCallback(cmdBuf); // call async callback
            vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuf); // free that command buffer
        });
    };

    template <class T> static inline auto makeVector(const T*ptr, size_t size = 1) { std::vector<T>v(size); memcpy(v.data(), ptr, strided<T>(size)); return v; };

    // create fence function
    static inline vk::Fence createFence(VkDevice device, bool signaled = true) {
        vk::FenceCreateInfo info = {};
        if (signaled) info.setFlags(vk::FenceCreateFlagBits::eSignaled);
        return vk::Device(device).createFence(info);
    };

    // List of all possible required extensions
    // Raytracing itself should support extension filtering and NVidia GPU alongside of RX Vega
    // Can work in single GPU systems (AMD or NVidia)
    static inline const std::vector<const char *> raytracingRequiredExtensions = {
        //"VK_AMD_gpu_shader_int16",
        //"VK_AMD_gpu_shader_half_float",
        //"VK_AMD_gpu_shader_half_float_fetch",
        "VK_AMD_buffer_marker",
        "VK_AMD_shader_info",
        "VK_AMD_texture_gather_bias_lod",
        "VK_AMD_shader_image_load_store_lod",
        "VK_AMD_gcn_shader",
        "VK_AMD_shader_trinary_minmax",
        "VK_KHR_8bit_storage",
        "VK_KHR_16bit_storage",
        "VK_KHR_descriptor_update_template",
        "VK_KHR_push_descriptor",
        "VK_KHR_image_format_list",
        "VK_KHR_sampler_mirror_clamp_to_edge",
        "VK_KHR_storage_buffer_storage_class",
        "VK_KHR_variable_pointers",
        "VK_KHR_relaxed_block_layout",
        "VK_KHR_get_memory_requirements2",
        "VK_KHR_bind_memory2",
        "VK_KHR_maintenance1",
        "VK_KHR_maintenance2",
        "VK_KHR_maintenance3",
        "VK_EXT_descriptor_indexing"
    };
};
