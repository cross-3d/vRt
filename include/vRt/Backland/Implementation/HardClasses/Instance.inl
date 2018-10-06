#pragma once

//#include "../../vRt_subimpl.inl"
#include "../Utils.hpp"

namespace _vt {
    using namespace vrt;

    // no such roles at now
    VtResult convertInstance(VkInstance vkInstance, VtInstanceConversionInfo vtInstanceCreateInfo, std::shared_ptr<Instance>& vtInstance) {
        vtInstance = std::make_shared<Instance>();
        vtInstance->_instance = vkInstance; // assign a Vulkan physical device
        return VK_SUCCESS;
    };
};
