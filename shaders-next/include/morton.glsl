#ifndef _MORTON_H
#define _MORTON_H

#ifdef USE_MORTON_32
#include "morton32x.glsl"
#else
#include "morton64x.glsl"
#endif

#endif