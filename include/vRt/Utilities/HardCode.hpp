#pragma once

#include "VkUtils.hpp"

namespace _vt {
    namespace hlbvh2 {

        inline static std::map<VtVendor, std::vector<uint32_t>> builder = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/bvh-build.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/bvh-build.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/bvh-build.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/bvh-build.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> fitBox = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/bvh-fit.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/bvh-fit.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/bvh-fit.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/bvh-fit.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> outerBox = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/bound-calc.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/bound-calc.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/bound-calc.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/bound-calc.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> genLeafs = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/leaf-gen.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/leaf-gen.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/leaf-gen.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/leaf-gen.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> linkLeafs = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/leaf-link.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/leaf-link.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/leaf-link.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/leaf-link.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> shorthand = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/shorthand.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/shorthand.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/shorthand.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/shorthand.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> traverse = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/traverse-bvh.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/traverse-bvh.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/traverse-bvh.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/traverse-bvh.comp.inl"
        } },
        };

        inline static std::map<VtVendor, std::vector<uint32_t>> interpolator = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/interpolator.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/interpolator.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/interpolator.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/interpolator.comp.inl"
        } },
        };
    };

    namespace qradix {
        inline static std::map<VtVendor, std::vector<uint32_t>> permute = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/qRadix/permute.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/qRadix/permute.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/qRadix/permute.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/qRadix/permute.comp.inl"
        } },
        };


        inline static std::map<VtVendor, std::vector<uint32_t>> workPrefix = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/qRadix/pfx-work.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/qRadix/pfx-work.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/qRadix/pfx-work.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/qRadix/pfx-work.comp.inl"
        } },
        };


        inline static std::map<VtVendor, std::vector<uint32_t>> histogram = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/qRadix/histogram.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/qRadix/histogram.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/qRadix/histogram.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/qRadix/histogram.comp.inl"
        } },
        };


    };
};