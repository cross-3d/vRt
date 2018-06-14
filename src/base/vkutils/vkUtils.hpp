#pragma once

#include "./vkStructures.hpp"

namespace NSM {

    // create fence function
    vk::Fence createFence(Device &device, bool signaled = true) {
        vk::FenceCreateInfo info;
        if (signaled) info.setFlags(vk::FenceCreateFlagBits::eSignaled);
        return device->logical.createFence(info);
    }

    // read source (unused)
    std::string readSource(const std::string &filePath, const bool &lineDirective = false) {
        std::string content;
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open())
        {
            std::cerr << "Could not read file " << filePath << ". File does not exist."
                << std::endl;
            return "";
        }
        std::string line = "";
        while (!fileStream.eof())
        {
            std::getline(fileStream, line);
            if (lineDirective || line.find("#line") == std::string::npos)
                content.append(line + "\n");
        }
        fileStream.close();
        return content;
    }

    // read binary (for SPIR-V)
    std::vector<char> readBinary(const std::string &filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        std::vector<char> data;
        if (file.is_open())
        {
            std::streampos size = file.tellg();
            data.resize(size);
            file.seekg(0, std::ios::beg);
            file.read(&data[0], size);
            file.close();
        }
        else
        {
            std::cerr << "Failure to open " + filePath << std::endl;
        }
        return data;
    };


    // load module for Vulkan device
    vk::ShaderModule loadAndCreateShaderModule(Device device, std::string path) {
        auto code = readBinary(path);
        return device->logical.createShaderModule(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), code.size(), (uint32_t *)code.data()));
    }


};
