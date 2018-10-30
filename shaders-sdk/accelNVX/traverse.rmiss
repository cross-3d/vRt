#version 460 core
#extension GL_NVX_raytracing : require
#extension GL_GOOGLE_include_directive : enable

#include "../include/driver.glsl"
#include "../include/mathlib.glsl"

struct VtCustomPayloadNVX {
    vec4 lastIntersection;
    uvec4 binaryData128;
};
#define LAST_INSTANCE primitiveState.binaryData128

layout(location = 0) rayPayloadInNVX VtCustomPayloadNVX primitiveState;

void main()
{
    primitiveState.lastIntersection = vec4(0.f.xx, INFINITY, FINT_ZERO);
};
