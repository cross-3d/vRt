#ifndef _STRUCTS_H
#define _STRUCTS_H

#include "../include/mathlib.glsl"

#ifdef USE_F32_BVH 
    #define UNPACK_LROW(m) uintBitsToFloat(m)
#else
    #ifdef AMD_F16_BVH
        #define UNPACK_LROW(m) unpackFloat4x16(m.xy)
    #else
        #define UNPACK_LROW(m) unpackHalf4x16(m.xy)
    #endif
#endif

#ifdef USE_F32_BVH
    #define UNPACK_RCRC(m) vec4(0.f)
#else
    #ifdef AMD_F16_BVH
        #define UNPACK_RCRC(m) unpackFloat4x16(m.zw)
    #else
        #define UNPACK_RCRC(m) unpackHalf4x16(m.zw)
    #endif
#endif


#ifdef USE_F32_BVH
    #define UNPACK_HF(m) vec4(0.f)
#else
    #ifdef AMD_F16_BVH
        #define UNPACK_HF(m) unpackFloat4x16(m)
    #else
        #define UNPACK_HF(m) unpackHalf4x16(m)
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
uvec2 writeColor(inout uvec2 rwby, in vec4 color){
    uint btw = BFE_HW(rwby.y, 16, 16);
    uvec2 clr = f32_f16(color);
    rwby = uvec2(clr.x, BFI_HW(clr.y, btw, 16, 16));
    return rwby;
}



// max attribute packing count
//const int ATTRIB_EXTENT = 8;
const int ATTRIB_EXTENT = 5;

struct VtHitData {
    int next; uint bitfield; int r0, r1;
    int attribID, rayID, payloadID, materialID;
    vec4 uvt, vdat; // UV, distance, triangle (base data), normal
};




// 16-bit offset of rays
const int B16FT = 16;

// ray bitfield
const ivec2 RAY_ACTIVED = ivec2(B16FT+0, 1);
const ivec2 RAY_TYPE = ivec2(B16FT+1, 2);
const ivec2 RAY_TARGET_LIGHT = ivec2(B16FT+3, 5);
const ivec2 RAY_BOUNCE = ivec2(B16FT+8, 3);
const ivec2 RAY_DBOUNCE = ivec2(B16FT+11, 3);
const ivec2 RAY_RAY_DL = ivec2(B16FT+14, 1);

// vertex bitfield
const ivec2 VTX_TYPE = ivec2(0, 2);
const ivec2 VTX_FRONT_FACE = ivec2(2, 1); // 0 is enabled, 1 is disabled
const ivec2 VTX_BACK_FACE = ivec2(3, 1); // 0 is enabled, 1 is disabled



int parameteri(const ivec2 parameter, in uint bitfield) {
    return int(BFE_HW(bitfield, parameter.x, parameter.y));
}

void parameteri(const ivec2 parameter, inout uint bitfield, in int value) {
    bitfield = BFI_HW(bitfield, uint(value), parameter.x, parameter.y);
}

lowp bool_ parameterb(const ivec2 parameter, in uint bitfield) {
    return bool_(BFE_HW(bitfield, parameter.x, 1));
}

void parameterb(const ivec2 parameter, inout uint bitfield, in lowp bool_ value) {
    bitfield = BFI_HW(bitfield, uint(value), parameter.x, 1);
}


int parameteri(const ivec2 parameter, in float bitfield) {
    return int(BFE_HW(floatBitsToUint(bitfield), parameter.x, parameter.y));
}

void parameteri(const ivec2 parameter, inout float bitfield, in int value) {
    bitfield = uintBitsToFloat(BFI_HW(floatBitsToUint(bitfield), uint(value), parameter.x, parameter.y));
}

lowp bool_ parameterb(const ivec2 parameter, in float bitfield) {
    return bool_(BFE_HW(floatBitsToUint(bitfield), parameter.x, 1));
}

void parameterb(const ivec2 parameter, inout float bitfield, in lowp bool_ value) {
    bitfield = uintBitsToFloat(BFI_HW(floatBitsToUint(bitfield), uint(value), parameter.x, 1));
}


struct bbox_t { vec4 mn, mx; };
struct leaf_t { bbox_t lbox; ivec4 pdata; };



#endif
