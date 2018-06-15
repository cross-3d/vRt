#pragma once

#include "../../vRt_subimpl.inl"

// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vt;


    constexpr uint32_t VtIdentifier = 0x1FFu;
    struct VkShortHead {
        //uint32_t sublevel : 24, identifier : 12;
        uint32_t identifier : 12, sublevel : 24; // I don't know ordering of bits
        uintptr_t pNext = 0ull; // pull of C++20 raw pointers
    };

    struct VkFullHead {
        uint32_t sType;
        uintptr_t pNext = 0ull; // pull of C++20 raw pointers
    };


    template <class VtS>
    auto vtSearchStructure(VtS& structure, VtStructureType sType) {
        VkFullHead* head = (VkFullHead*)&structure;
        VkFullHead* found = nullptr;
        for (int i = 0; i < 255; i++) {
            if (!head) break;
            if (head->sType == sType) {
                found = head; break;
            }
            head = (VkFullHead*)head->pNext;
        }
        return found;
    };

    template <class VtS>
    auto vtExplodeArtificals(VtS& structure) {
        VkShortHead* head = (VkShortHead*)&structure;
        VkShortHead* lastVkStructure = nullptr;
        VkShortHead* firstVkStructure = nullptr;
        for (int i = 0; i < 255; i++) {
            if (!head) break;
            if (head->identifier != VtIdentifier) {
                if (lastVkStructure) {
                    lastVkStructure->pNext = (uintptr_t)head;
                } else {
                    firstVkStructure = head;
                }
                lastVkStructure = head;
            }
            head = (VkShortHead*)head->pNext;
        }
        return firstVkStructure;
    };

    // transition texture layout
    inline auto imageBarrier(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> image) {
        VtResult result = VK_SUCCESS; // planned to complete
        if (image->_initialLayout == image->_layout) return result; // no need transfering more

        vk::ImageMemoryBarrier imageMemoryBarriers = {};
        imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.oldLayout = (vk::ImageLayout)image->_initialLayout;
        imageMemoryBarriers.newLayout = (vk::ImageLayout)image->_layout;
        imageMemoryBarriers.image = image->_image;
        imageMemoryBarriers.subresourceRange = image->_subresourceRange;

        // Put barrier on top
        vk::PipelineStageFlags srcStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
        vk::PipelineStageFlags dstStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
        vk::DependencyFlags dependencyFlags{};
        vk::AccessFlags srcMask{};
        vk::AccessFlags dstMask{};

        typedef vk::ImageLayout il;
        typedef vk::AccessFlagBits afb;

        // Is it me, or are these the same?
        switch ((vk::ImageLayout)image->_initialLayout) {
            case il::eUndefined: break;
            case il::eGeneral: srcMask = afb::eTransferWrite; break;
            case il::eColorAttachmentOptimal: srcMask = afb::eColorAttachmentWrite; break;
            case il::eDepthStencilAttachmentOptimal: srcMask = afb::eDepthStencilAttachmentWrite; break;
            case il::eDepthStencilReadOnlyOptimal: srcMask = afb::eDepthStencilAttachmentRead; break;
            case il::eShaderReadOnlyOptimal: srcMask = afb::eShaderRead; break;
            case il::eTransferSrcOptimal: srcMask = afb::eTransferRead; break;
            case il::eTransferDstOptimal: srcMask = afb::eTransferWrite; break;
            case il::ePreinitialized: srcMask = afb::eTransferWrite | afb::eHostWrite; break;
            case il::ePresentSrcKHR: srcMask = afb::eMemoryRead; break;
        }

        switch ((vk::ImageLayout)image->_layout) {
            case il::eUndefined: break;
            case il::eGeneral: dstMask = afb::eTransferWrite; break;
            case il::eColorAttachmentOptimal: dstMask = afb::eColorAttachmentWrite; break;
            case il::eDepthStencilAttachmentOptimal: dstMask = afb::eDepthStencilAttachmentWrite; break;
            case il::eDepthStencilReadOnlyOptimal: dstMask = afb::eDepthStencilAttachmentRead; break;
            case il::eShaderReadOnlyOptimal: dstMask = afb::eShaderRead; break;
            case il::eTransferSrcOptimal: dstMask = afb::eTransferRead; break;
            case il::eTransferDstOptimal: dstMask = afb::eTransferWrite; break;
            case il::ePreinitialized: dstMask = afb::eTransferWrite; break;
            case il::ePresentSrcKHR: dstMask = afb::eMemoryRead; break;
        }

        // assign access masks
        imageMemoryBarriers.srcAccessMask = srcMask;
        imageMemoryBarriers.dstAccessMask = dstMask;

        // barrier
        vk::CommandBuffer(cmd).pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlagBits::eByRegion, {}, {}, std::array<vk::ImageMemoryBarrier, 1>{imageMemoryBarriers});
        image->_initialLayout = (VkImageLayout)imageMemoryBarriers.newLayout;

        return result;
    };




    // copy buffer command with inner "DeviceBuffer"
    inline void cmdCopyBuffer(VkCommandBuffer cmd, std::shared_ptr<DeviceBuffer> srcBuffer, std::shared_ptr<DeviceBuffer> dstBuffer, const std::vector<vk::BufferCopy>& regions) {
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions);
    };

    // copy to host buffer
    // you can't use it for form long command buffer to host
    inline void cmdCopyBufferToHost(VkCommandBuffer cmd, std::shared_ptr<DeviceBuffer> srcBuffer, std::shared_ptr<DeviceToHostBuffer> dstBuffer, const std::vector<vk::BufferCopy>& regions) {
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions, toHostCommandBarrier);
    };

    // copy from host buffer
    // you can't use it for form long command buffer from host
    inline void cmdCopyBufferFromHost(VkCommandBuffer cmd, std::shared_ptr<HostToDeviceBuffer> srcBuffer, std::shared_ptr<DeviceBuffer> dstBuffer, const std::vector<vk::BufferCopy>& regions) {
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions, fromHostCommandBarrier);
    };



    inline void cmdCopyDeviceImage(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::ImageCopy>& regions) {
        //vk::CommandBuffer(cmd).copyBufferToImage((vk::Buffer&)(srcBuffer->_buffer), (vk::Image&)(dstImage->_image), vk::ImageLayout(dstImage->_layout), regions);
        vk::CommandBuffer(cmd).copyImage((vk::Image&)(*srcImage), vk::ImageLayout(srcImage->_layout), (vk::Image&)(*dstImage), vk::ImageLayout(dstImage->_layout), regions);
        commandBarrier(cmd);
    };

    // copy buffer to image (from gpu or host)
    // you can't use it for form long command buffer from host
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_CPU_TO_GPU>
    inline void cmdCopyBufferToImage(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> srcBuffer, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::BufferImageCopy>& regions) {
        vk::CommandBuffer(cmd).copyBufferToImage((vk::Buffer&)(*srcBuffer), (vk::Image&)(*dstImage), vk::ImageLayout(dstImage->_layout), regions);

        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { fromHostCommandBarrier(cmd); } else {
            if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) { toHostCommandBarrier(cmd); } else { commandBarrier(cmd); }
        }
    };

    // copy image to buffer (to gpu or host)
    // you can't use it for form long command buffer to host
    template<VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_TO_CPU>
    inline void cmdCopyImageToBuffer(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<RoledBuffer<U>> dstBuffer, const std::vector<vk::BufferImageCopy>& regions) {
        vk::CommandBuffer(cmd).copyImageToBuffer((vk::Image&)(*srcImage), vk::ImageLayout(srcImage->_layout), (vk::Buffer&)(*dstBuffer), regions);

        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { fromHostCommandBarrier(cmd); } else {
            if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) { toHostCommandBarrier(cmd); } else { commandBarrier(cmd); }
        }
    };


    template<VmaMemoryUsage U>
    using cmdCopyImageToBuffer_T = void(*)(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<RoledBuffer<U>> dstBuffer, const std::vector<vk::BufferImageCopy>& regions);

    template<VmaMemoryUsage U>
    using cmdCopyBufferToImage_T = void(*)(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> srcBuffer, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::BufferImageCopy>& regions);

    // aliased low level calls
    constexpr cmdCopyBufferToImage_T<VMA_MEMORY_USAGE_GPU_ONLY> copyDeviceBufferToImage = &cmdCopyBufferToImage<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr cmdCopyImageToBuffer_T<VMA_MEMORY_USAGE_GPU_ONLY> copyImageToDeviceBuffer = &cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_ONLY>;
    constexpr cmdCopyBufferToImage_T<VMA_MEMORY_USAGE_CPU_TO_GPU> copyHostBufferToImage = &cmdCopyBufferToImage<VMA_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr cmdCopyImageToBuffer_T<VMA_MEMORY_USAGE_GPU_TO_CPU> copyImageToHostBuffer = &cmdCopyImageToBuffer<VMA_MEMORY_USAGE_GPU_TO_CPU>;



    // direct state commands for loading data

    // set buffer data function (defaultly from HostToDevice)
    template <class T, VmaMemoryUsage U = VMA_MEMORY_USAGE_CPU_TO_GPU>
    inline void setBufferSubData(const std::vector<T> &hostdata, std::shared_ptr<RoledBuffer<U>> buffer, intptr_t offset = 0) {
        const size_t bufferSize = hostdata.size() * sizeof(T);
        if (bufferSize > 0) memcpy((uint8_t *)buffer->_hostMapped() + offset, hostdata.data(), bufferSize);
    }

    // get buffer data function (defaultly from DeviceToHost)
    template <class T, VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_TO_CPU>
    inline void getBufferSubData(std::shared_ptr<RoledBuffer<U>> buffer, std::vector<T> &hostdata, intptr_t offset = 0) {
        if (hostdata.size() > 0) memcpy(hostdata.data(), (const uint8_t *)buffer->_hostMapped() + offset, hostdata.size() * sizeof(T));
    }

    // get buffer data function (defaultly from DeviceToHost)
    template <class T, VmaMemoryUsage U = VMA_MEMORY_USAGE_GPU_TO_CPU>
    inline auto getBufferSubData(std::shared_ptr<RoledBuffer<U>> buffer, size_t count = 1, intptr_t offset = 0) {
        std::vector<T> hostdata(count);
        if (hostdata.size() > 0) memcpy(hostdata.data(), (const uint8_t *)buffer->_hostMapped() + offset, count * sizeof(T));
        return hostdata; // in return will copying, C++ does not made mechanism for zero-copy of anything
    }

};
