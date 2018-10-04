#pragma once

// implementables
#include "../../../vRt/vRt.h"
#include "../Definitions/HardClasses.inl"


// C++ internal initializers for hard classes
namespace _vt { // store in undercover namespace
    using namespace vrt;


    constexpr const  uint32_t VtIdentifier = 0x1FFu;
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
    inline auto vtSearchStructure(VtS& structure, VtStructureType sType) {
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
    inline auto vtExplodeArtificals(VtS& structure) {
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

#ifndef VRT_ENABLE_VEZ_INTEROP
        if (image->_initialLayout == image->_layout) return result; // no need transfering more

        vk::ImageMemoryBarrier imageMemoryBarriers = {};
        imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers.oldLayout = (vk::ImageLayout)image->_initialLayout;
        imageMemoryBarriers.newLayout = (vk::ImageLayout)image->_layout;
        imageMemoryBarriers.image = image->_image;
        imageMemoryBarriers.subresourceRange = image->_subresourceRange;

        // Put barrier on top
        vk::PipelineStageFlags srcStageMask{ vk::PipelineStageFlagBits::eBottomOfPipe };
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
        vk::CommandBuffer(cmd).pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, std::array<vk::ImageMemoryBarrier, 1>{imageMemoryBarriers});
#endif
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




    // copy buffer command with inner "DeviceBuffer"
    inline void cmdCopyBuffer(VkCommandBuffer cmd, std::shared_ptr<BufferRegion> srcBuffer, std::shared_ptr<BufferRegion> dstBuffer, std::vector<vk::BufferCopy> regions) {
        regions[0].srcOffset += srcBuffer->_offset(), regions[0].dstOffset += dstBuffer->_offset();
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions);
    };

    // copy to host buffer
    // you can't use it for form long command buffer to host
    inline void cmdCopyBufferToHost(VkCommandBuffer cmd, std::shared_ptr<BufferRegion> srcBuffer, std::shared_ptr<DeviceToHostBuffer> dstBuffer, std::vector<vk::BufferCopy> regions) {
        regions[0].srcOffset += srcBuffer->_offset();
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions, toHostCommandBarrier);
    };

    // copy from host buffer
    // you can't use it for form long command buffer from host
    inline void cmdCopyBufferFromHost(VkCommandBuffer cmd, std::shared_ptr<HostToDeviceBuffer> srcBuffer, std::shared_ptr<BufferRegion> dstBuffer, std::vector<vk::BufferCopy> regions) {
        regions[0].dstOffset += dstBuffer->_offset();
        cmdCopyBufferL(cmd, VkBuffer(*srcBuffer), VkBuffer(*dstBuffer), regions, fromHostCommandBarrier);
    };




    inline void cmdCopyDeviceImage(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::ImageCopy>& regions) {
        //vk::CommandBuffer(cmd).copyBufferToImage((vk::Buffer&)(srcBuffer->_buffer), (vk::Image&)(dstImage->_image), vk::ImageLayout(dstImage->_layout), regions);
        vk::CommandBuffer(cmd).copyImage((vk::Image&)(*srcImage), vk::ImageLayout(srcImage->_layout), (vk::Image&)(*dstImage), vk::ImageLayout(dstImage->_layout), regions);
        commandBarrier(cmd);
    };

    // copy buffer to image (from gpu or host)
    // you can't use it for form long command buffer from host
    template<VtMemoryUsage U = VT_MEMORY_USAGE_CPU_TO_GPU>
    inline void cmdCopyBufferToImage(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> srcBuffer, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::BufferImageCopy>& regions) {
        vk::CommandBuffer(cmd).copyBufferToImage(srcBuffer->_buffer, dstImage->_image, vk::ImageLayout(dstImage->_layout), regions);

        if constexpr (U == VT_MEMORY_USAGE_CPU_TO_GPU) { fromHostCommandBarrier(cmd); } else {
            if constexpr (U == VT_MEMORY_USAGE_GPU_TO_CPU) { toHostCommandBarrier(cmd); } else { commandBarrier(cmd); }
        }
    };

    // copy image to buffer (to gpu or host)
    // you can't use it for form long command buffer to host
    template<VtMemoryUsage U = VT_MEMORY_USAGE_GPU_TO_CPU>
    inline void cmdCopyImageToBuffer(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<RoledBuffer<U>> dstBuffer, const std::vector<vk::BufferImageCopy>& regions) {
        vk::CommandBuffer(cmd).copyImageToBuffer((vk::Image&)(*srcImage), vk::ImageLayout(srcImage->_layout), (vk::Buffer&)(*dstBuffer), regions);

        if constexpr (U == VT_MEMORY_USAGE_CPU_TO_GPU) { fromHostCommandBarrier(cmd); } else {
            if constexpr (U == VT_MEMORY_USAGE_GPU_TO_CPU) { toHostCommandBarrier(cmd); } else { commandBarrier(cmd); }
        }
    };


    template<VtMemoryUsage U>
    using cmdCopyImageToBuffer_T = void(*)(VkCommandBuffer cmd, std::shared_ptr<DeviceImage> srcImage, std::shared_ptr<RoledBuffer<U>> dstBuffer, const std::vector<vk::BufferImageCopy>& regions);

    template<VtMemoryUsage U>
    using cmdCopyBufferToImage_T = void(*)(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> srcBuffer, std::shared_ptr<DeviceImage> dstImage, const std::vector<vk::BufferImageCopy>& regions);

    // aliased low level calls
    constexpr const  cmdCopyBufferToImage_T<VT_MEMORY_USAGE_GPU_ONLY> copyDeviceBufferToImage = &cmdCopyBufferToImage<VT_MEMORY_USAGE_GPU_ONLY>;
    constexpr const  cmdCopyImageToBuffer_T<VT_MEMORY_USAGE_GPU_ONLY> copyImageToDeviceBuffer = &cmdCopyImageToBuffer<VT_MEMORY_USAGE_GPU_ONLY>;
    constexpr const  cmdCopyBufferToImage_T<VT_MEMORY_USAGE_CPU_TO_GPU> copyHostBufferToImage = &cmdCopyBufferToImage<VT_MEMORY_USAGE_CPU_TO_GPU>;
    constexpr const  cmdCopyImageToBuffer_T<VT_MEMORY_USAGE_GPU_TO_CPU> copyImageToHostBuffer = &cmdCopyImageToBuffer<VT_MEMORY_USAGE_GPU_TO_CPU>;



    // direct state commands for loading data

    // set buffer data function (defaultly from HostToDevice)
    template <class T, VtMemoryUsage U = VT_MEMORY_USAGE_CPU_TO_GPU>
    inline void setBufferSubData(const std::vector<T> &hostdata, std::shared_ptr<RoledBuffer<U>> buffer, intptr_t offset = 0) {
        const VkDeviceSize bufferSize = hostdata.size() * sizeof(T);
#ifdef VRT_ENABLE_VEZ_INTEROP
        if (bufferSize > 0) {
            uint8_t * uPtr = nullptr;
            vezMapBuffer(*buffer->_device, buffer->_buffer, offset, bufferSize, (void**)&uPtr);
            memcpy(uPtr, hostdata.data(), bufferSize);

            auto region = VezMappedBufferRange{};
            region.buffer = buffer->_buffer;
            region.offset = offset;
            region.size = bufferSize;
            //vezFlushMappedBufferRanges(*buffer->_device, 1, &region);
            vezUnmapBuffer(*buffer->_device, buffer->_buffer);
        }
#else
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        if (bufferSize > 0) memcpy((uint8_t *)buffer->_hostMapped() + offset, hostdata.data(), bufferSize);
#endif
#endif
    }

    // get buffer data function (defaultly from DeviceToHost)
    template <class T, VtMemoryUsage U = VT_MEMORY_USAGE_GPU_TO_CPU>
    inline void getBufferSubData(std::shared_ptr<RoledBuffer<U>> buffer, std::vector<T> &hostdata, intptr_t offset = 0) {
        const VkDeviceSize bufferSize = hostdata.size() * sizeof(T);
#ifdef VRT_ENABLE_VEZ_INTEROP
        if (bufferSize > 0) {
            uint8_t * uPtr = nullptr;
            vezMapBuffer(*buffer->_device, buffer->_buffer, offset, bufferSize, &uPtr);
            memcpy(hostdata.data(), uPtr, bufferSize);

            auto region = VezMappedBufferRange{};
            region.buffer = buffer->_buffer;
            region.offset = offset;
            region.size = bufferSize;
            //vezFlushMappedBufferRanges(*buffer->_device, 1, &region);
            vezUnmapBuffer(*buffer->_device, buffer->_buffer);
        }
#else
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        if (hostdata.size() > 0) memcpy(hostdata.data(), (const uint8_t *)buffer->_hostMapped() + offset, bufferSize);
#endif
#endif
    }

    // get buffer data function (defaultly from DeviceToHost)
    template <class T, VtMemoryUsage U = VT_MEMORY_USAGE_GPU_TO_CPU>
    inline auto getBufferSubData(std::shared_ptr<RoledBuffer<U>> buffer, VkDeviceSize count = 1, intptr_t offset = 0) {
        std::vector<T> hostdata(count);
        getBufferSubData(buffer, hostdata, 0);
        return hostdata; // in return will copying, C++ does not made mechanism for zero-copy of anything
    };

    inline auto getVendorName(const uint32_t& vendorID) {
        auto shaderDir = VT_VENDOR_UNIVERSAL;
        switch (vendorID) {
        case 4318:
            shaderDir = VT_VENDOR_NVIDIA;
            break;
        case 4098:
            shaderDir = VT_VENDOR_AMD;
            break;
        case 32902:
            shaderDir = VT_VENDOR_INTEL;
            break;
        }
        return shaderDir;
    };

    // short data set with command buffer (alike push constant)
    template<class T, VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY>
    static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> dstBuffer, VkDeviceSize offset, const std::vector<T>& data) {
        return cmdUpdateBuffer<T>(cmd, *dstBuffer, offset, data);
    };

    // short data set with command buffer (alike push constant)
    template<class T, VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY>
    static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> dstBuffer, VkDeviceSize offset, const VkDeviceSize& size, const T*data) {
        return cmdUpdateBuffer<T>(cmd, *dstBuffer, offset, size, data);
    };

    // short data set with command buffer (alike push constant)
    template<class T>
    static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, std::shared_ptr<BufferRegion> dstBuffer, VkDeviceSize offset, const std::vector<T>& data) {
        return cmdUpdateBuffer<T>(cmd, *dstBuffer, offset + dstBuffer->_offset(), data);
    };

    // short data set with command buffer (alike push constant)
    template<class T>
    static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, std::shared_ptr<BufferRegion> dstBuffer, VkDeviceSize offset, const VkDeviceSize& size, const T*data) {
        return cmdUpdateBuffer<T>(cmd, *dstBuffer, offset + dstBuffer->_offset(), size, data);
    };



    template<uint32_t Rv, VtMemoryUsage U = VT_MEMORY_USAGE_GPU_ONLY>
    static inline VkResult cmdFillBuffer(VkCommandBuffer cmd, std::shared_ptr<RoledBuffer<U>> dstBuffer, VkDeviceSize size = VK_WHOLE_SIZE, intptr_t offset = 0) {
        return cmdFillBuffer<Rv>(cmd, *dstBuffer, std::min(dstBuffer->_size(), size), offset);
    };

    template<uint32_t Rv>
    static inline VkResult cmdFillBuffer(VkCommandBuffer cmd, std::shared_ptr<BufferRegion> dstBuffer, VkDeviceSize size = VK_WHOLE_SIZE, intptr_t offset = 0) {
        return cmdFillBuffer<Rv>(cmd, *dstBuffer, std::min(dstBuffer->_size(), size), offset + dstBuffer->_offset());
    };

};


// templates is not supported by static libs
// all pure C++ stuff will implementing by headers in SDK
namespace vrt {
    template <class T>
    inline VtResult vtSetBufferSubData(const std::vector<T> &hostdata, VtHostToDeviceBuffer buffer, VkDeviceSize offset) {
        _vt::setBufferSubData<T, VT_MEMORY_USAGE_CPU_TO_GPU>(hostdata, buffer, offset); return VK_SUCCESS;
    };

    template <class T>
    inline VtResult vtGetBufferSubData(VtDeviceToHostBuffer buffer, std::vector<T> &hostdata, VkDeviceSize offset) {
        _vt::getBufferSubData<T, VT_MEMORY_USAGE_GPU_TO_CPU>(buffer, hostdata, offset); return VK_SUCCESS;
    };

    template <class T>
    inline std::vector<T> vtGetBufferSubData(VtDeviceToHostBuffer buffer, VkDeviceSize count, VkDeviceSize offset) {
        return _vt::getBufferSubData<T, VT_MEMORY_USAGE_GPU_TO_CPU>(buffer, count, offset);
    };
};
