#version 460 core
#extension GL_NV_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable

#define IS_RAY_SHADER
#include "../include/driver.glsl"
#include "../include/mathlib.glsl"

struct VtCustomPayload {
     vec4 lastIntersection;
    ivec4 binaryData128;
};
#define LAST_INSTANCE primitiveState.binaryData128

layout(location = 0) rayPayloadInNV VtCustomPayload primitiveState;

void main()
{
    primitiveState.lastIntersection = vec4(0.f.xx, INFINITY, FINT_ZERO);
    primitiveState.binaryData128 = (-1).xxxx;
    //primitiveState.lastIntersection = vec4(0.f.xx, INFINITY, intBitsToFloat(1));
};
