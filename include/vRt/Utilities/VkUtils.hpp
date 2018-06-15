#pragma once

//#include "../Parts/Headers.inl"

#include <vulkan/vulkan.hpp> // only for inner usage
#include <vulkan/vk_mem_alloc.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <array>
#include <map>
#include <random>
#include <vector>
#include <algorithm>
#include <execution>
#include <iterator>
#include <cstddef>
#include <optional>

namespace _vt {
    constexpr auto DEFAULT_FENCE_TIMEOUT = 100000000000ll;

    template <typename T>
    inline auto sgn(T val) { return (T(0) < val) - (val < T(0)); }

    template<class T = uint32_t>
    inline T tiled(T sz, T gmaxtile) {
        // return (int32_t)ceil((double)sz / (double)gmaxtile);
        return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
    }


    inline double milliseconds() {
        auto duration = std::chrono::high_resolution_clock::now();
        double millis = std::chrono::duration_cast<std::chrono::nanoseconds>(
            duration.time_since_epoch())
            .count() /
            1000000.0;
        return millis;
    }

    template <class T>
    inline size_t strided(size_t sizeo) { return sizeof(T) * sizeo; }






    // read binary (for SPIR-V)
    inline auto readBinary(std::string filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        std::vector<uint8_t> data;
        if (file.is_open()) {
            std::streampos size = file.tellg();
            data.resize(size);
            file.seekg(0, std::ios::beg);
            file.read((char *)data.data(), size);
            file.close();
        } else {
            std::cerr << "Failure to open " + filePath << std::endl;
        }
        return data;
    };

    // read source (unused)
    inline std::string readSource(const std::string &filePath, const bool &lineDirective = false) {
        std::string content = "";
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open()) {
            std::cerr << "Could not read file " << filePath << ". File does not exist."
                << std::endl;
            return "";
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
    inline void commandBarrier(const VkCommandBuffer& cmdBuffer) {
        VkMemoryBarrier memoryBarrier;
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr);
    };


    // from host command buffer barrier
    inline void fromHostCommandBarrier(const VkCommandBuffer& cmdBuffer) {
        VkMemoryBarrier memoryBarrier;
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr);
    };


    // to host command buffer barrier
    inline void toHostCommandBarrier(const VkCommandBuffer& cmdBuffer) {
        VkMemoryBarrier memoryBarrier;
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT | VK_PIPELINE_STAGE_HOST_BIT,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr);
    };


    // create secondary command buffers for batching compute invocations
    inline auto createCommandBuffer(const VkDevice device, const VkCommandPool cmdPool, bool secondary = true, bool once = true) {
        VkCommandBuffer cmdBuffer = nullptr;

        VkCommandBufferAllocateInfo cmdi;
        cmdi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdi.pNext = nullptr;
        cmdi.commandPool = cmdPool;
        cmdi.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdi.commandBufferCount = 1;
        vkAllocateCommandBuffers(device, &cmdi, &cmdBuffer);

        VkCommandBufferInheritanceInfo inhi;
        inhi.pNext = nullptr;
        inhi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inhi.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

        VkCommandBufferBeginInfo bgi;
        bgi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bgi.pNext = nullptr;
        bgi.flags = {};
        bgi.flags = once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        bgi.pInheritanceInfo = secondary ? &inhi : nullptr;
        vkBeginCommandBuffer(cmdBuffer, &bgi);

        return cmdBuffer;
    };


    inline auto loadAndCreateShaderModuleInfo(const std::vector<uint8_t>& code) {
        VkShaderModuleCreateInfo smi;
        smi.pNext = nullptr;
        smi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        smi.pCode = (uint32_t *)code.data();
        smi.codeSize = code.size();
        smi.flags = {};
        return smi;
    }

    // create shader module
    inline auto loadAndCreateShaderModule(VkDevice device, const std::vector<uint8_t>& code) {
        VkShaderModule sm = nullptr;
        vkCreateShaderModule(device, &loadAndCreateShaderModuleInfo(code), nullptr, &sm);
        return sm;
    };

    // create shader module
    inline auto loadAndCreateShaderModuleStage(VkDevice device, const std::vector<uint8_t>& code, const char * entry = "main") {
        VkPipelineShaderStageCreateInfo spi;
        spi.pNext = nullptr;
        spi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        spi.flags = {};
        spi.module = loadAndCreateShaderModule(device, code);
        spi.pName = entry;
        spi.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        spi.pSpecializationInfo = nullptr;
        return spi;
    };

    // create compute pipelines
    inline auto createCompute(VkDevice device, std::string path, VkPipelineLayout layout, VkPipelineCache cache) {
        auto code = readBinary(path);
        auto spi = loadAndCreateShaderModuleStage(device, code);

        VkComputePipelineCreateInfo cmpi;
        cmpi.pNext = nullptr;
        cmpi.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        cmpi.flags = {};
        cmpi.layout = layout;
        cmpi.stage = spi;
        cmpi.basePipelineHandle = {};
        cmpi.basePipelineIndex = -1;

        VkPipeline pipeline = nullptr;
        vkCreateComputePipelines(device, cache, 1, &cmpi, nullptr, &pipeline);
        return pipeline;
    }

    // create compute pipelines
    inline auto createCompute(VkDevice device, const VkPipelineShaderStageCreateInfo& spi, VkPipelineLayout layout, VkPipelineCache cache) {
        VkComputePipelineCreateInfo cmpi;
        cmpi.pNext = nullptr;
        cmpi.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        cmpi.flags = {};
        cmpi.layout = layout;
        cmpi.stage = spi;
        cmpi.basePipelineHandle = {};
        cmpi.basePipelineIndex = -1;

        VkPipeline pipeline = nullptr;
        vkCreateComputePipelines(device, cache, 1, &cmpi, nullptr, &pipeline);
        return pipeline;
    }

    // add dispatch in command buffer (with default pipeline barrier)
    inline VkResult cmdDispatch(VkCommandBuffer cmd, VkPipeline pipeline, uint32_t x = 1, uint32_t y = 1, uint32_t z = 1) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdDispatch(cmd, x, y, z);
        commandBarrier(cmd); // put shader barrier
        return VK_SUCCESS;
    }

    // low level copy command between (prefer for host and device)
    inline VkResult cmdCopyBufferL(VkCommandBuffer cmd, vk::Buffer srcBuffer, vk::Buffer dstBuffer, const std::vector<vk::BufferCopy>& regions, const std::function<void(const VkCommandBuffer&)>& barrierFn = commandBarrier) {
        vk::CommandBuffer(cmd).copyBuffer(srcBuffer, dstBuffer, regions);
        barrierFn(cmd); // put copy barrier
        return VK_SUCCESS;
    }

    // short data set with command buffer (alike push constant)
    template<class T>
    inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, const std::vector<T>& data, vk::Buffer dstBuffer, VkDeviceSize offset = 0) {
        vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, data);
        //commandBarrier(cmd);
        fromHostCommandBarrier(cmd);
        return VK_SUCCESS;
    }

    // template function for fill buffer by constant value
    // use for create repeat variant
    template<uint32_t Rv>
    inline VkResult cmdFillBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize size = VK_WHOLE_SIZE, intptr_t offset = 0) {
        vk::CommandBuffer(cmd).fillBuffer(vk::Buffer(dstBuffer), offset, size, Rv);
        //commandBarrier(cmd);
        fromHostCommandBarrier(cmd);
        return VK_SUCCESS;
    }

    // make whole size buffer descriptor info
    inline auto bufferDescriptorInfo(vk::Buffer buffer, vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE) {
        return vk::DescriptorBufferInfo(buffer, offset, size);
    }




    // submit command (with async wait)
    inline void submitCmd(VkDevice device, VkQueue queue, std::vector<VkCommandBuffer> cmds, vk::SubmitInfo smbi = {}) {
        // no commands 
        if (cmds.size() <= 0) return;

        smbi.commandBufferCount = cmds.size();
        smbi.pCommandBuffers = (vk::CommandBuffer *)cmds.data();

        VkFence fence = nullptr; VkFenceCreateInfo fin{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr };
        vkCreateFence(device, &fin, nullptr, &fence);
        vkQueueSubmit(queue, 1, (const VkSubmitInfo *)&smbi, fence);
        vkWaitForFences(device, 1, &fence, true, DEFAULT_FENCE_TIMEOUT);
        vkDestroyFence(device, fence, nullptr);
    };

    // once submit command buffer
    inline void submitOnce(VkDevice device, VkQueue queue, VkCommandPool cmdPool, std::function<void(VkCommandBuffer)> cmdFn = {}, vk::SubmitInfo smbi = {}) {
        auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf);
        submitCmd(device, queue, { cmdBuf });
        vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuf); // free that command buffer
    };


    // submit command (with async wait)
    inline void submitCmdAsync(VkDevice device, VkQueue queue, std::vector<VkCommandBuffer> cmds, std::function<void()> asyncCallback = {}, vk::SubmitInfo smbi = {}) {
        // no commands 
        if (cmds.size() <= 0) return;

        smbi.commandBufferCount = cmds.size();
        smbi.pCommandBuffers = (const vk::CommandBuffer *)cmds.data();

        VkFence fence = nullptr; VkFenceCreateInfo fin{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr };
        vkCreateFence(device, &fin, nullptr, &fence);
        vkQueueSubmit(queue, 1, (const VkSubmitInfo *)&smbi, fence);
        std::async(std::launch::async | std::launch::deferred, [=]() {
            vkWaitForFences(device, 1, &fence, true, DEFAULT_FENCE_TIMEOUT);
            std::async(std::launch::async | std::launch::deferred, [=]() {
                vkDestroyFence(device, fence, nullptr);
                if (asyncCallback) asyncCallback();
            });
        });
    };

    // once submit command buffer
    inline void submitOnceAsync(VkDevice device, VkQueue queue, VkCommandPool cmdPool, std::function<void(VkCommandBuffer)> cmdFn = {}, std::function<void(VkCommandBuffer)> asyncCallback = {}, vk::SubmitInfo smbi = {}) {
        auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf);
        submitCmdAsync(device, queue, { cmdBuf }, [=]() {
            asyncCallback(cmdBuf); // call async callback
            vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuf); // free that command buffer
        });
    };




    template <class T>
    inline auto makeVector(const T*ptr, size_t size = 1) {
        std::vector<T>v(size); memcpy(v.data(), ptr, strided<T>(size));
        return v;
    }



    // create fence function
    inline vk::Fence createFence(VkDevice device, bool signaled = true) {
        vk::FenceCreateInfo info;
        if (signaled) info.setFlags(vk::FenceCreateFlagBits::eSignaled);
        return vk::Device(device).createFence(info);
    }


};
