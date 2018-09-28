#pragma once

#include "vRt_subimpl.inl"

// implement these
#ifdef VRT_ENABLE_HARDCODED_SPV_CORE
#include "Utilities/HardCode.hpp"
#else
#include "Utilities/SpvPaths.hpp"
#endif

// implementations 
#include "Implementation/API/HandlersMethods.inl" // in-handler accessing implementation
#include "Implementation/API/APIImpl.inl" // import API implementation
#include "Implementation/HardClassesImpl.inl" // import internal classes
