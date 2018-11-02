#pragma once

#include "VkUtils.hpp"

namespace _vt {
    using namespace vrt;
    
    static inline const std::array<std::string, 6> pathNames{ "universal", "amd", "nvidia", "intel", "vega", "turing" };

    namespace hlbvh2 {
        static inline constexpr const auto builderFirst = "hlBVH2/bvh-build-first.comp";
        static inline constexpr const auto builder      = "hlBVH2/bvh-build.comp";
        static inline constexpr const auto fitBox       = "hlBVH2/bvh-fit.comp";
        static inline constexpr const auto linkLeafs    = "hlBVH2/leaf-link.comp";
        static inline constexpr const auto shorthand    = "hlBVH2/shorthand.comp";
        static inline constexpr const auto traverse     = "hlBVH2/traverse-bvh.comp";
        static inline constexpr const auto interpolator = "hlBVH2/interpolator.comp";
        static inline constexpr const auto outerBox     = "hlBVH2/bound-calc.comp";
        
        namespace triangle {
            static inline constexpr const auto boxCalc = "hlBVH2/triangle/box-calc.comp";
            static inline constexpr const auto genLeaf = "hlBVH2/triangle/leaf-gen.comp";
        };

        namespace instance {
            static inline constexpr const auto boxCalc = "hlBVH2/AABB/box-calc.comp";
            static inline constexpr const auto genLeaf = "hlBVH2/AABB/leaf-gen.comp";
        };
    };
    
    namespace qradix {
        static inline constexpr const auto permute = "radix/permute.comp";
        static inline constexpr const auto workPrefix = "radix/pfx-work.comp";
        static inline constexpr const auto histogram = "radix/histogram.comp";
        static inline constexpr const auto copyhack = "radix/copyhack.comp";
    };

    namespace natives {
        static inline constexpr const auto vertexAssembly = "native/vinput.comp";
    };

    static inline const auto getCorrectPath(const std::string& fpath = "", const VtVendor& vendor = VT_VENDOR_UNIVERSAL, const std::string& directory = "./intrusive") {
        return (directory + "/" + pathNames[vendor] + "/" + fpath + ".spv");
    };

};
