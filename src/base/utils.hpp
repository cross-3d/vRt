#pragma once

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

#define GLM_FORCE_SWIZZLE

//#define PCG_LITTLE_ENDIAN 0
#include <half.hpp> // force include half's
#include <pcg_random.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

#ifdef USE_CIMG
#include "tinyexr.h"
#define cimg_plugin "CImg/tinyexr_plugin.hpp"
//#define cimg_use_png
//#define cimg_use_jpeg
#include "CImg.h"
#endif

#ifndef NSM
#define NSM ste
#endif

namespace NSM {
    template <typename T>
    inline auto sgn(T val) { return (T(0) < val) - (val < T(0)); }

    template<class T = uint32_t>
    static inline T tiled(T sz, T gmaxtile) {
        return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
    }

    static inline double milliseconds() {
        auto duration = std::chrono::high_resolution_clock::now();
        double millis = std::chrono::duration_cast<std::chrono::nanoseconds>(
            duration.time_since_epoch())
            .count() /
            1000000.0;
        return millis;
    }

    template <class T>
    inline size_t strided(size_t sizeo) { return sizeof(T) * sizeo; }
}
