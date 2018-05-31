#ifndef _UNIFORMS_H
#define _UNIFORMS_H

#include "../include/structs.glsl"

#define RAY_BLOCK rayBlock. 
#define GEOMETRY_BLOCK geometryBlock.  

struct MaterialUniformStruct {
    int materialOffset;
    int materialCount;
    int phase;
    int lightcount;
};

struct SamplerUniformStruct {
    vec2 sceneRes;
    int iterationCount;
    int blockCount;
    int blockBinCount;
    int texelCount;
    int hitCount;
    int traverseCount;
    mat4 transform;
};

struct LightUniformStruct {
    vec4 lightVector;
    vec4 lightColor;
    vec4 lightOffset;
    vec4 lightAmbient;
    vec4 lightRandomizedOrigin;
};

struct GeometryUniformStruct {
    mat4x4 transform;
    mat4x4 transformInv;
    mat4x4 projection; 
    mat4x4 projectionInv;

    int materialID;
    int triangleCount;
    int triangleOffset;
    int clearDepth;
};

struct CameraUniformStruct {
    mat4x4 projInv;
    mat4x4 camInv;
    mat4x4 prevCamInv;

    float ftime;
    int enable360;
    int interlace;
    int interlaceStage;
};


struct RayStream {
    vec4 diffuseStream;
    uvec2 superseed[4];
    vec4 frand4;
};


layout ( std430, binding = 12, set = 0 ) readonly buffer lightUniformB {
    LightUniformStruct lightNode[];
} lightUniform;

layout ( std430, binding = 13, set = 0 ) readonly buffer rayBlockB {
    SamplerUniformStruct samplerUniform;
    CameraUniformStruct cameraUniform;
    MaterialUniformStruct materialUniform;
} rayBlock; 

layout ( std430, binding = 14, set = 0 ) readonly buffer rayStreamsB {
    RayStream rayStreams[];
};

layout ( std430, binding = 16, set = 0 ) readonly buffer shuffledSequenceB {
    uint shuffledSequence[];
};

#endif
