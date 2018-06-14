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

#include <misc/half.hpp> // force include half's
#include <misc/pcg_random.hpp>
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
#define NSM vte
#endif
