#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <unistd.h>
//#include <dos.h>
//#include <windows.h>

#include <misc/args.hxx>
#include <misc/half.hpp>
#include <misc/pcg_random.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <cmath>
#include <cfenv>
#include <ios>
#include <sstream>

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

// include ray tracing library
#include "vRt/vRt.h"

// include extensions
//#include "vRt/vRtX/RTXAcceleratorExtension.hpp"

// include VMA for compatibility
#include <vulkan/vk_mem_alloc.h>

// include inner utils from library (for development purpose)
#include "vRt/Backland/Utilities/VkUtils.hpp"

// also need include these headers
#include "vRt/Backland/Implementation/Utils.hpp"

// inner utils of application
namespace NSM {

    

};
