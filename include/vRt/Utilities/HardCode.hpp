#pragma once

#include "VkUtils.hpp"

namespace _vt {
    namespace hlbvh2 {

        inline static const std::map<VtVendor, std::vector<uint32_t>> builderFirst = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/hlBVH2/bvh-build-first.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/hlBVH2/bvh-build-first.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/hlBVH2/bvh-build-first.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/hlBVH2/bvh-build-first.comp.inl"
        } },
        };

        inline static const std::map<VtVendor, std::vector<uint32_t>> builder = {
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

        inline static const std::map<VtVendor, std::vector<uint32_t>> fitBox = {
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



        namespace triangle {

            inline static const std::map<VtVendor, std::vector<uint32_t>> outerBox = {
                { VT_VENDOR_AMD,{
    #include "../HardCodes/amd/hlBVH2/triangle/bound-calc.comp.inl"
            } },
            { VT_VENDOR_NVIDIA,{
    #include "../HardCodes/nvidia/hlBVH2/triangle/bound-calc.comp.inl"
            } },
            { VT_VENDOR_INTEL,{
    #include "../HardCodes/intel/hlBVH2/triangle/bound-calc.comp.inl"
            } },
            { VT_VENDOR_UNIVERSAL,{
    #include "../HardCodes/universal/hlBVH2/triangle/bound-calc.comp.inl"
            } },
            };

            inline static const std::map<VtVendor, std::vector<uint32_t>> genLeafs = {
                { VT_VENDOR_AMD,{
    #include "../HardCodes/amd/hlBVH2/triangle/leaf-gen.comp.inl"
            } },
            { VT_VENDOR_NVIDIA,{
    #include "../HardCodes/nvidia/hlBVH2/triangle/leaf-gen.comp.inl"
            } },
            { VT_VENDOR_INTEL,{
    #include "../HardCodes/intel/hlBVH2/triangle/leaf-gen.comp.inl"
            } },
            { VT_VENDOR_UNIVERSAL,{
    #include "../HardCodes/universal/hlBVH2/triangle/leaf-gen.comp.inl"
            } },
            };

        };



        inline static const std::map<VtVendor, std::vector<uint32_t>> linkLeafs = {
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

        inline static const std::map<VtVendor, std::vector<uint32_t>> shorthand = {
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

        inline static const std::map<VtVendor, std::vector<uint32_t>> traverse = {
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

        inline static const std::map<VtVendor, std::vector<uint32_t>> interpolator = {
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
        inline static const std::map<VtVendor, std::vector<uint32_t>> permute = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/radix/permute.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/radix/permute.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/radix/permute.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/radix/permute.comp.inl"
        } },
        };


        inline static const std::map<VtVendor, std::vector<uint32_t>> workPrefix = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/radix/pfx-work.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/radix/pfx-work.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/radix/pfx-work.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/radix/pfx-work.comp.inl"
        } },
        };


        inline static const std::map<VtVendor, std::vector<uint32_t>> histogram = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/radix/histogram.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/radix/histogram.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/radix/histogram.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/radix/histogram.comp.inl"
        } },
        };


        inline static const std::map<VtVendor, std::vector<uint32_t>> copyhack = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/radix/copyhack.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/radix/copyhack.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/radix/copyhack.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/radix/copyhack.comp.inl"
        } },
        };

    };

    namespace natives {

        inline static const std::map<VtVendor, std::vector<uint32_t>> vertexAssembly = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/native/vinput.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/native/vinput.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/native/vinput.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/native/vinput.comp.inl"
        } },
        };


        inline static const std::map<VtVendor, std::vector<uint32_t>> dullBarrier = {
            { VT_VENDOR_AMD,{
#include "../HardCodes/amd/native/dull.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/native/dull.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/native/dull.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/native/dull.comp.inl"
        } },
        };


        inline static const std::map<VtVendor, std::vector<uint32_t>> triplet = {
    { VT_VENDOR_AMD,{
#include "../HardCodes/amd/native/triplet.comp.inl"
        } },
        { VT_VENDOR_NVIDIA,{
#include "../HardCodes/nvidia/native/triplet.comp.inl"
        } },
        { VT_VENDOR_INTEL,{
#include "../HardCodes/intel/native/triplet.comp.inl"
        } },
        { VT_VENDOR_UNIVERSAL,{
#include "../HardCodes/universal/native/triplet.comp.inl"
        } },
        };

    }
};
