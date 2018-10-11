#pragma once

#include "VkUtils.hpp"

namespace _vt {
    using namespace vrt;
    
    namespace hlbvh2 {

        static inline const std::map<VtVendor, std::string> builderFirst = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/bvh-build-first.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/bvh-build-first.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/bvh-build-first.comp.spv" },
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/bvh-build-first.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> builder = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/bvh-build.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/bvh-build.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/bvh-build.comp.spv" },
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/bvh-build.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> fitBox = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/bvh-fit.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/bvh-fit.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/bvh-fit.comp.spv" },
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/bvh-fit.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> linkLeafs = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/leaf-link.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/leaf-link.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/leaf-link.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/leaf-link.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> shorthand = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/shorthand.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/shorthand.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/shorthand.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/shorthand.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> traverse = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/traverse-bvh.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/traverse-bvh.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/traverse-bvh.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/traverse-bvh.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> interpolator = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/interpolator.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/interpolator.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/interpolator.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/interpolator.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> outerBox = {
            { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/bound-calc.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/bound-calc.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/bound-calc.comp.spv" },
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/bound-calc.comp.spv" }
        };

        namespace triangle {
            static inline const std::map<VtVendor, std::string> boxCalc = {
                { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/triangle/box-calc.comp.spv" },
                { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/triangle/box-calc.comp.spv" },
                { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/triangle/box-calc.comp.spv" },
                { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/triangle/box-calc.comp.spv" }
            };

            static inline const std::map<VtVendor, std::string> genLeafs = {
                { VT_VENDOR_AMD, "./intrusive/amd/hlBVH2/triangle/leaf-gen.comp.spv" },
                { VT_VENDOR_NVIDIA, "./intrusive/nvidia/hlBVH2/triangle/leaf-gen.comp.spv" },
                { VT_VENDOR_INTEL, "./intrusive/intel/hlBVH2/triangle/leaf-gen.comp.spv" }, 
                { VT_VENDOR_UNIVERSAL, "./intrusive/universal/hlBVH2/triangle/leaf-gen.comp.spv" }
            };
        };
    };
    
    namespace qradix {
        static inline const std::map<VtVendor, std::string> permute = {
            { VT_VENDOR_AMD, "./intrusive/amd/radix/permute.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/radix/permute.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/radix/permute.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/radix/permute.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> workPrefix = {
            { VT_VENDOR_AMD, "./intrusive/amd/radix/pfx-work.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/radix/pfx-work.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/radix/pfx-work.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/radix/pfx-work.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> histogram = {
            { VT_VENDOR_AMD, "./intrusive/amd/radix/histogram.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/radix/histogram.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/radix/histogram.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/radix/histogram.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> copyhack = {
            { VT_VENDOR_AMD, "./intrusive/amd/radix/copyhack.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/radix/copyhack.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/radix/copyhack.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/radix/copyhack.comp.spv" }
        };
    };

    namespace natives {
        static inline const std::map<VtVendor, std::string> vertexAssembly = {
            { VT_VENDOR_AMD, "./intrusive/amd/native/vinput.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/native/vinput.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/native/vinput.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/native/vinput.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> dullBarrier = {
            { VT_VENDOR_AMD, "./intrusive/amd/native/dull.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/native/dull.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/native/dull.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/native/dull.comp.spv" }
        };

        static inline const std::map<VtVendor, std::string> triplet = {
            { VT_VENDOR_AMD, "./intrusive/amd/native/triplet.comp.spv" },
            { VT_VENDOR_NVIDIA, "./intrusive/nvidia/native/triplet.comp.spv" },
            { VT_VENDOR_INTEL, "./intrusive/intel/native/triplet.comp.spv" }, 
            { VT_VENDOR_UNIVERSAL, "./intrusive/universal/native/triplet.comp.spv" }
        };
    };
};
