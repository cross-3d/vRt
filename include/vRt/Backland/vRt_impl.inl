#pragma once

#include "vRt_subimpl.inl"

// implement these
#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
#include "Utilities/HardCode.hpp"
#else
#include "Utilities/SpvPaths.hpp"
#endif

// implementations 
#include "Backland/Definitions/HandlersMethods.inl" // in-handler accessing implementation
#include "Implementation/HardClassesImpl.inl" // import internal classes
#include "Implementation/APIImpl.inl" // import API implementation
