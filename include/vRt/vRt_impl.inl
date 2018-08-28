#pragma once

#include "vRt_subimpl.inl"

// implement these
#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
#include "Utilities/HardCode.hpp"
#else
#include "Utilities/SpvPaths.hpp"
#endif

#include "Implementation/HardClasses.inl"
#include "Implementation/API/Calls.inl"
