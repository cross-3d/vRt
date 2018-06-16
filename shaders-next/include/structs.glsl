#ifndef _STRUCTS_H
#define _STRUCTS_H

#include "../include/mathlib.glsl"

struct Texel {
     vec4 coord;
     vec4 color; // when collected from blocks
     vec4 p3d;
     vec4 albedo;
     vec4 normal;
};

struct bbox {
     vec4 mn;
     vec4 mx;
};



#ifdef USE_F32_BVH 
    #define UNPACK_LROW(m) uintBitsToFloat(m)
#else
    #ifdef AMD_F16_BVH
        #define UNPACK_LROW(m) unpackFloat16x4(m.xy)
    #else
        #define UNPACK_LROW(m) unpackHalf16x4(m.xy)
    #endif
#endif

#ifdef USE_F32_BVH
    #define UNPACK_RCRC(m) vec4(0.f)
#else
    #ifdef AMD_F16_BVH
        #define UNPACK_RCRC(m) unpackFloat16x4(m.zw)
    #else
        #define UNPACK_RCRC(m) unpackHalf16x4(m.zw)
    #endif
#endif


#ifdef USE_F32_BVH
    #define UNPACK_HF(m) vec4(0.f)
#else
    #ifdef AMD_F16_BVH
        #define UNPACK_HF(m) unpackFloat16x4(m)
    #else
        #define UNPACK_HF(m) unpackHalf16x4(m)
    #endif
#endif







// ray bitfield spec (reduced to 16-bit)
// {0     }[1] - actived or not
// {1 ..2 }[2] - ray type (for example diffuse, specular, shadow)
// {3 ..7 }[5] - stream/light system id
// {8 ..10}[3] - bounce index, 3-bit (reflection, refraction)
// {11..13}[3] - bounce index, 3-bit (diffuse)
// {14    }[1] - can lighten by sunlights


// possible structure in 32-byte presentation 
// but: 
// 1. use cartesian direction instead of 3-vec
// 2. need resolve "vec4" conflict of 16-bit encoded data in "vec3", because need reserve one of 32-bit 
// 3. decline of "final" color term 
// 4. reduce bitfield to 16-bit weight 

// possible extra solutions: 
// 1. add extra array of ray metadata/offload (but we very limited in binding registers)

struct VtRay {
     vec4 origin; vec2 cdirect; uvec2 dcolor;
};



// write color, but don't write (save) last element
uvec2 WriteColor(inout uvec2 rwby, in vec4 color){
    uint btw = BFE_HW(rwby.y, 16, 16);
    uvec2 clr = f32_f16(color);
    rwby = uvec2(clr.x, BFI_HW(clr.y, btw, 16, 16));
    return rwby;
}



// max attribute packing count
const int ATTRIB_EXTENT = 8;

// attribute formating
const int NORMAL_TID = 0;
const int TEXCOORD_TID = 1;
const int TANGENT_TID = 2;
const int BITANGENT_TID = 3;
const int VCOLOR_TID = 4;

struct HitData {
    vec4 uvt; // UV, distance, triangle (base data)
    int rayID; // ray index
    int payloadID; // hit shaded index
    int materialID; // may not necessary 
    int next; // next chainged hit

    vec4 attributes[ATTRIB_EXTENT];
};

struct HitPayload {
    // hit shaded data
    vec4 normalHeight;
    vec4 albedo;
    vec4 emission;
    vec4 specularGlossiness;
};



const int B16FT = 16;
const ivec2 ACTIVED = ivec2(B16FT+0, 1);
const ivec2 TYPE = ivec2(B16FT+1, 2);
const ivec2 TARGET_LIGHT = ivec2(B16FT+3, 5);
const ivec2 BOUNCE = ivec2(B16FT+8, 3);
const ivec2 DBOUNCE = ivec2(B16FT+11, 3);
const ivec2 RAY_DL = ivec2(B16FT+14, 1);




int parameteri(const ivec2 parameter, inout uint bitfield) {
    return int(BFE_HW(bitfield, parameter.x, parameter.y));
}

void parameteri(const ivec2 parameter, inout uint bitfield, in int value) {
    bitfield = BFI_HW(bitfield, uint(value), parameter.x, parameter.y);
}

bool_ parameterb(const ivec2 parameter, inout uint bitfield) {
    return bool_(BFE_HW(bitfield, parameter.x, 1));
}

void parameterb(const ivec2 parameter, inout uint bitfield, in bool_ value) {
    bitfield = BFI_HW(bitfield, uint(value), parameter.x, 1);
}


int parameteri(const ivec2 parameter, inout float bitfield) {
    return int(BFE_HW(floatBitsToUint(bitfield), parameter.x, parameter.y));
}

void parameteri(const ivec2 parameter, inout float bitfield, in int value) {
    bitfield = uintBitsToFloat(BFI_HW(floatBitsToUint(bitfield), uint(value), parameter.x, parameter.y));
}

bool_ parameterb(const ivec2 parameter, inout float bitfield) {
    return bool_(BFE_HW(floatBitsToUint(bitfield), parameter.x, 1));
}

void parameterb(const ivec2 parameter, inout float bitfield, in bool_ value) {
    bitfield = uintBitsToFloat(BFI_HW(floatBitsToUint(bitfield), uint(value), parameter.x, 1));
}


#define RAY_BITFIELD_ ray.dcolor.y


bool_ RayActived(inout VtRay ray) {
    return parameterb(ACTIVED, RAY_BITFIELD_);
}

void RayActived(inout VtRay ray, in bool_ actived) {
    parameterb(ACTIVED, RAY_BITFIELD_, actived);
}




int RayType(inout VtRay ray) {
    return parameteri(TYPE, RAY_BITFIELD_);
}

void RayType(inout VtRay ray, in int type) {
    parameteri(TYPE, RAY_BITFIELD_, type);
}



// restore law about direct light and caustics
bool_ RayDL(inout VtRay ray) {
    return parameterb(RAY_DL, RAY_BITFIELD_);
}

void RayDL(inout VtRay ray, in bool_ dl) {
    parameterb(RAY_DL, RAY_BITFIELD_, dl);
}



int RayTargetLight(inout VtRay ray) {
    return parameteri(TARGET_LIGHT, RAY_BITFIELD_);
}

void RayTargetLight(inout VtRay ray, in int tl) {
    parameteri(TARGET_LIGHT, RAY_BITFIELD_, tl);
}


int RayBounce(inout VtRay ray) {
    return int(uint(parameteri(BOUNCE, RAY_BITFIELD_)));
}

void RayBounce(inout VtRay ray, in int bn) {
    parameteri(BOUNCE, RAY_BITFIELD_, int(uint(bn)));
}


int RayDiffBounce(inout VtRay ray) {
    return int(uint(parameteri(DBOUNCE, RAY_BITFIELD_)));
}

void RayDiffBounce(inout VtRay ray, in int bn) {
    parameteri(DBOUNCE, RAY_BITFIELD_, int(uint(bn)));
}



struct HlbvhNode {
     vec4 lbox[2];
     ivec4 pdata;
};



#endif
