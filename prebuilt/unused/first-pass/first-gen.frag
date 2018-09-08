#version 460 core
#extension GL_GOOGLE_include_directive : enable

#define SETS_DESC_SET_ID 3
#define VRT_USE_FAST_INTERSECTION
#define USE_SINGLE_THREAD_RAY_MANAGMENT
#define SIMPLIFIED_RAY_MANAGMENT
#define DISCARD_SHARED_CACHING
#define ENABLE_TRAVERSE_DATA
#define ENABLE_VSTORAGE_DATA
#define DMA_HIT


#include "../include/driver.glsl"
#include "../include/mathlib.glsl"
#include "../include/ballotlib.glsl"
#include "../include/structs.glsl"
#include "../include/rayslib.glsl"
#include "../include/morton.glsl"
#include "../include/vertex.glsl"
#include "../include/hitlib.glsl"


layout (binding = 0, set = 4, std430) readonly buffer VT_GEN_EXAMPLE {
    mat4x4 camInv, camT;
    mat4x4 projInv, projT;
    vec4 sceneRes;
    int enable360, variant, r1, r2;
} cameraUniform;

// in-interpolatable
layout (binding = 0) out vec4 origin;
layout (binding = 1) out vec4 normal;
layout (binding = 2) out vec4 texcoord;
layout (binding = 3) out vec4 tangent;
layout (binding = 4) out vec4 bitangent;
layout (binding = 5) out vec4 vcolor;
layout (binding = 6) out flat int materialID;


void main(){
    
}
