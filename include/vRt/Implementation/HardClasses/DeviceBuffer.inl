#pragma once
#include "../../vRt_subimpl.inl"

namespace _vt {
    using namespace vrt;


    // destructor of advanced buffer
    template<VmaMemoryUsage U>
    inline RoledBuffer<U>::~RoledBuffer() {
        std::async([=]() {
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezDestroyBuffer(_device->_device, _buffer);
#else
            vmaDestroyBuffer(_device->_allocator, _buffer, _allocation);
#endif
        });
    };

    /*
    DeviceBuffer::~RoledBuffer() {
        std::async([=]() {
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezDestroyBuffer(_device->_device, _buffer);
#else
            vmaDestroyBuffer(_device->_allocator, _buffer, _allocation);
#endif
        });
    };

    HostToDeviceBuffer::~RoledBuffer() {
        std::async([=]() {
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezDestroyBuffer(_device->_device, _buffer);
#else
            vmaDestroyBuffer(_device->_allocator, _buffer, _allocation);
#endif
        });
    };

    DeviceToHostBuffer::~RoledBuffer() {
        std::async([=]() {
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezDestroyBuffer(_device->_device, _buffer);
#else
            vmaDestroyBuffer(_device->_allocator, _buffer, _allocation);
#endif
        });
    };
    */

    VtResult createBufferView(std::shared_ptr<BufferRegion> bRegion) {
        auto device = bRegion->_device;
        VtResult result = VK_SUCCESS;

        if (bRegion->_size() < sizeof(uint32_t)) return VK_ERROR_INITIALIZATION_FAILED;
        if (result == VK_SUCCESS && bRegion->_format) {
            //bRegion->_bufferView = {};

#ifdef VRT_ENABLE_VEZ_INTEROP
            auto bvi = VezBufferViewCreateInfo{};
#else
            auto bvi = VkBufferViewCreateInfo(vk::BufferViewCreateInfo{});
            bvi.flags = {};
#endif
            bvi.pNext = nullptr;
            bvi.buffer = VkBuffer(*bRegion);
            bvi.format = bRegion->_format;
            bvi.offset = bRegion->_offset();
            bvi.range = bRegion->_size();

#ifdef VRT_ENABLE_VEZ_INTEROP
            if (vezCreateBufferView(device->_device, &bvi, &bRegion->_bufferView()) == VK_SUCCESS) {
#else
            if (vkCreateBufferView(device->_device, &bvi, nullptr, &bRegion->_bufferView()) == VK_SUCCESS) {
#endif
                result = VK_SUCCESS;
            }
            else {
                result = VK_INCOMPLETE;
            };
        };
        return result;
    };


    template<VmaMemoryUsage U>
    VtResult createBuffer(std::shared_ptr<Device> device, VtDeviceBufferCreateInfo cinfo, std::shared_ptr<RoledBuffer<U>>& vtDeviceBuffer) {
        VtResult result = VK_ERROR_INITIALIZATION_FAILED;

        //auto vtDeviceBuffer = (_vtBuffer = std::make_shared<RoledBuffer<U>>());
        vtDeviceBuffer = std::make_shared<RoledBuffer<U>>();
        vtDeviceBuffer->_device = device; // delegate device by weak_ptr

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = U;

        // make memory usages 
        auto usageFlagCstr = 0u;
        if constexpr (U != VMA_MEMORY_USAGE_GPU_ONLY) { allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; }
        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) { usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; } else {
            if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) { usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; } else {
                usageFlagCstr |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            };
        };

        auto usageFlag = cinfo.usageFlag | usageFlagCstr;
        //if (cinfo.format) { usageFlag |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; } // if has format, add texel storage usage
        usageFlag |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

#ifdef VRT_ENABLE_VEZ_INTEROP
        VezMemoryFlags mem = VEZ_MEMORY_GPU_ONLY;
        if constexpr (U == VMA_MEMORY_USAGE_CPU_TO_GPU) mem = VEZ_MEMORY_CPU_TO_GPU;
        if constexpr (U == VMA_MEMORY_USAGE_GPU_TO_CPU) mem = VEZ_MEMORY_GPU_TO_CPU;
        if constexpr (U == VMA_MEMORY_USAGE_CPU_ONLY) mem = VEZ_MEMORY_CPU_ONLY;

        auto binfo = VezBufferCreateInfo{};
        binfo.pNext = nullptr;
#else
        auto binfo = VkBufferCreateInfo(vk::BufferCreateInfo{});
        binfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
#endif

        //binfo.queueFamilyIndexCount = 1;
        //binfo.pQueueFamilyIndices = &cinfo.familyIndex;
        binfo.size = cinfo.bufferSize;//((cinfo.bufferSize >> 5ull) << 5ull) + 32ull;
        binfo.usage = usageFlag;

#ifdef VRT_ENABLE_VEZ_INTEROP
        result = vezCreateBuffer(device->_device, mem, &binfo, &vtDeviceBuffer->_buffer);
#else
        result = vmaCreateBuffer(device->_allocator, &binfo, &allocCreateInfo, &vtDeviceBuffer->_buffer, &vtDeviceBuffer->_allocation, &vtDeviceBuffer->_allocationInfo);
#endif

        // if format is known, make bufferView
        if constexpr (U == VMA_MEMORY_USAGE_GPU_ONLY) {
            VtBufferRegionCreateInfo rbc = {};
            rbc.offset = 0, rbc.bufferSize = cinfo.bufferSize, rbc.format = cinfo.format;
            createBufferRegion(vtDeviceBuffer, rbc, vtDeviceBuffer->_bufferRegion);
        }

        return result;
    };


    // create shared buffer, buffer views and resolve descriptor info (with externalization support)
    inline VtResult createSharedBuffer(std::shared_ptr<BufferManager> bManager, std::shared_ptr<DeviceBuffer>& gBuffer, VtDeviceBufferCreateInfo cinfo) {
        cinfo.bufferSize = bManager->_size; createDeviceBuffer(bManager->_device, cinfo, bManager->_bufferStore); gBuffer = bManager->_bufferStore;

        // complete descriptors and buffer-views
        for (auto&f : bManager->_bufferRegions) {
            f->_descriptorInfo().buffer = *(f->_boundBuffer = std::weak_ptr(bManager->_bufferStore)).lock(); createBufferView(f);
        }

        // return result (TODO: handling)
        return VK_SUCCESS;
    };


    // create shared buffer, buffer views and resolve descriptor info
    inline VtResult createSharedBuffer(std::shared_ptr<BufferManager> bManager, VtDeviceBufferCreateInfo cinfo) {
        cinfo.bufferSize = bManager->_size; createDeviceBuffer(bManager->_device, cinfo, bManager->_bufferStore);

        // complete descriptors and buffer-views
        for (auto&f : bManager->_bufferRegions) {
            f->_descriptorInfo().buffer = *(f->_boundBuffer = std::weak_ptr(bManager->_bufferStore)).lock(); createBufferView(f);
        }

        // return result (TODO: handling)
        return VK_SUCCESS;
    };


    // create buffer manager
    inline VtResult createBufferManager(std::shared_ptr<Device> gDevice, std::shared_ptr<BufferManager>& bManager) {
        bManager = std::make_shared<BufferManager>();
        bManager->_device = gDevice;
        return VK_SUCCESS;
    };

    // create buffer region by exist buffer
    inline VtResult createBufferRegion(std::shared_ptr<DeviceBuffer> gBuffer, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion) {
        auto gDevice = gBuffer->_device;
        auto correctedSize = bri.bufferSize;//((bri.bufferSize >> 5ull) << 5ull) + 32ull;
        bRegion = std::make_shared<BufferRegion>();
        bRegion->_device = gDevice;
        bRegion->_format = bri.format;
        bRegion->_descriptorInfo().range = correctedSize;
        bRegion->_descriptorInfo().offset = bri.offset;
        bRegion->_descriptorInfo().buffer = *(bRegion->_boundBuffer = std::weak_ptr(gBuffer)).lock(); createBufferView(bRegion);
        return VK_SUCCESS;
    };

    // create structuring 
    inline VtResult BufferManager::_prealloc(VtBufferRegionCreateInfo cinfo, std::shared_ptr<BufferRegion>& bRegion) {
        auto correctedSize = ((cinfo.bufferSize >> 5ull) << 5ull) + 32ull, offset = _size; _size += correctedSize;
        _bufferRegions.push_back(std::make_shared<BufferRegion>());
        bRegion = _bufferRegions[_bufferRegions.size() - 1];
        bRegion->_device = _device;
        bRegion->_format = cinfo.format;
        bRegion->_descriptorInfo().range = correctedSize;
        bRegion->_descriptorInfo().offset = offset;
        return VK_SUCCESS;
    };

    // create buffer region by buffer manager
    inline VtResult createBufferRegion(std::shared_ptr<BufferManager> bManager, VtBufferRegionCreateInfo bri, std::shared_ptr<BufferRegion>& bRegion) {
        return bManager->_prealloc(bri, bRegion);
    };

};
