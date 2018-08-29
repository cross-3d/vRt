#ifndef _MORTON_H
#define _MORTON_H

// generative of few papers, no direct equivalent
// may used in http://dcgi.felk.cvut.cz/projects/emc/emc2017.pdf

#ifdef USE_MORTON_32
#include "morton32x.glsl"
#else
#include "morton64x.glsl"
#endif

#endif