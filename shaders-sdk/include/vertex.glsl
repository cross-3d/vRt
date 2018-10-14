#ifndef _VERTEX_H
#define _VERTEX_H

#include "../include/mathlib.glsl"


#ifdef ENABLE_VERTEX_INTERPOLATOR
#ifndef ENABLE_VSTORAGE_DATA
#define ENABLE_VSTORAGE_DATA
#endif
#endif



// Geometry Zone
#if (defined(ENABLE_VSTORAGE_DATA) || defined(VERTEX_FILLING))

    #ifndef VTX_SET
        #ifdef VERTEX_FILLING
        #define VTX_SET 0
        #else
        #define VTX_SET 2
        #endif
    #endif


    #ifdef VERTEX_FILLING
    layout ( binding = 2, set = VTX_SET, std430   ) buffer bitfieldsB { uint vbitfields[]; };
    #else
    layout ( binding = 2, set = VTX_SET, std430   ) readonly buffer bitfieldsB { int vbitfields[]; };
    #endif


    #if (defined(LEAF_GEN) || defined(VERTEX_FILLING))
    layout ( binding = 5, set = VTX_SET, rgba32f ) coherent uniform imageBuffer lvtx;
    layout ( binding = 7, set = VTX_SET, rgba32f ) coherent uniform imageBuffer lnrm;
    #else
    layout ( binding = 5, set = VTX_SET, rgba32f ) readonly uniform imageBuffer lvtx;
    layout ( binding = 7, set = VTX_SET, rgba32f ) readonly uniform imageBuffer lnrm;
    #endif
    
#endif


// task level traverse data 
struct BvhBlockT {
    int entryID, leafCount; // leafCount reserved for ESC versions
    int primitiveCount, primitiveOffset; // first for default triangle limit, second for correct vertex data block ( because assembled set should have shared buffer )
    mat4x4 transform, transformInv; // we resolved to save that row 
    vec4 sceneMin, sceneMax;
};

// TODO: remerge to new top level traverser 
struct BvhInstanceT {
    int bvhBlockID, r0, r1, r2;
    mat4x4 transform, transformIn; // row of traversion correction, combined with transforming to instance space 
};



// required 64-byte per full node 
struct BTYPE_ {
#if (defined(USE_F32_BVH) || defined(USE_F16_BVH)) && !defined(EXPERIMENTAL_UNORM16_BVH)
    fvec4_ cbox[3];
#else
    uvec2 cbox[3];
#endif
#if (defined(USE_F16_BVH) || defined(EXPERIMENTAL_UNORM16_BVH) || !defined(USE_F32_BVH))
    uvec2 spacing[3]; // when using 16-bit data, need have data space ()
#endif
    ivec4 meta;
};


#ifndef VERTEX_FILLING
// Block of main BVH structure (for bottom levels will not required)
layout ( binding = 0, set = 1, std430 ) readonly restrict buffer bvhBlockB { BvhBlockT bvhBlock_[]; }; // bvhBlock of main structure 


// Accessible blocks and instances for top levels, or task accessing (required shared buffers)
#ifdef EXPERIMENTAL_INSTANCING_SUPPORT
#ifdef BVH_CREATION
layout ( binding = 2, set = 1, std430 ) restrict buffer BvhInstanceB { BvhInstanceT bvhInstance_[]; };
layout ( binding = 3, set = 1, std430 ) restrict buffer bvhBlockInB { BvhBlockT bvhBlockIn_[]; };
#else
layout ( binding = 2, set = 1, std430 ) readonly restrict buffer BvhInstanceB { BvhInstanceT bvhInstance_[]; };
layout ( binding = 3, set = 1, std430 ) readonly restrict buffer bvhBlockInB { BvhBlockT bvhBlockIn_[]; };
#endif
#endif


#ifdef BVH_CREATION
layout ( binding = 1, set = 1, std430 )          restrict buffer bvhBoxesB { BTYPE_ bvhNodes[]; };
#else
layout ( binding = 1, set = 1, std430 ) readonly restrict buffer bvhBoxesB { BTYPE_ bvhNodes[]; };
#endif
#endif


#ifdef EXPERIMENTAL_INSTANCING_SUPPORT
int INSTANCE_ID = 0;
#define bvhInstance bvhInstance_[INSTANCE_ID]
#define bvhBlockIn bvhBlockIn_[bvhInstance.bvhBlockID]
#define bvhBlockTop bvhBlock_[0] 
#else
#define bvhBlockIn bvhBlock_[0] 
#define bvhBlockTop bvhBlock_[0] 
#endif

const uint BVH_STATE_TOP = 0, BVH_STATE_BOTTOM = 1;

#define BVH_ENTRY bvhBlockIn.entryID
#define BVH_ENTRY_HALF (bvhBlockIn.entryID>>1)





#ifndef VERTEX_FILLING
//vec4 uniteBox(in vec4 glb) { return fma((glb - vec4(bvhBlock.sceneMin.xyz, 0.f)) / vec4((bvhBlock.sceneMax.xyz - bvhBlock.sceneMin.xyz), 1.f), vec4( 2.f.xxx,  1.f), vec4(-1.f.xxx, 0.f)); };
vec4 uniteBox(in vec4 glb) { return point4(fma((glb - bvhBlockIn.sceneMin) / (bvhBlockIn.sceneMax - bvhBlockIn.sceneMin), 2.f.xxxx, -1.f.xxxx), glb.w); };
vec4 uniteBoxTop(in vec4 glb) { return point4(fma((glb - bvhBlockTop.sceneMin) / (bvhBlockTop.sceneMax - bvhBlockTop.sceneMin), 2.f.xxxx, -1.f.xxxx), glb.w); };
#endif

const mat3 uvwMap = mat3(vec3(1.f,0.f,0.f),vec3(0.f,1.f,0.f),vec3(0.f,0.f,1.f));



#ifndef VERTEX_FILLING
#ifndef BVH_CREATION
#ifdef ENABLE_VSTORAGE_DATA

#ifndef VRT_USE_FAST_INTERSECTION
float intersectTriangle(in vec4 orig, in mat3 M, in int axis, in int tri, inout vec2 UV, in bool _valid) {
    float T = INFINITY;
    IFANY (_valid) {
        const mat3 ABC = mat3((TLOAD(lvtx, tri*3+0)+orig.x).xyz, (TLOAD(lvtx, tri*3+1)+orig.y).xyz, (TLOAD(lvtx, tri*3+2)+orig.z).xyz)*M;

        // watertight triangle intersection (our, GPU-GLSL adapted version)
        // http://jcgt.org/published/0002/01/05/paper.pdf
        vec3 UVW_ = uvwMap[axis] * inverse(ABC);
        IFANY ((all(greaterThan(UVW_, 0.f.xxx)) || all(lessThan(UVW_, 0.f.xxx))) && _valid) {
            UVW_ /= (dot(UVW_, 1.f.xxx));
            UV = vec2(UVW_.yz), UVW_ *= ABC; // calculate axis distances
            T = mix(mix(UVW_.z, UVW_.y, axis == 1), UVW_.x, axis == 0);
            [[flatten]] if ( T < (-SFN) || T >= N_INFINITY ) _valid = false;
        }
    }
    return (_valid ? T : INFINITY);
}
#endif


#ifdef VRT_USE_FAST_INTERSECTION
float intersectTriangle(in vec4 orig, in vec4 dir, in int tri, inout vec2 uv, in bool _valid, in float cdist) {
    float T = INFINITY;
    IFANY (_valid) {
#ifdef VTX_USE_LEGACY_METHOD
        // classic intersection (Möller–Trumbore)
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        const mat3 vT = mat3(TLOAD(lvtx, tri*3+0).xyz, TLOAD(lvtx, tri*3+1).xyz, TLOAD(lvtx, tri*3+2).xyz);
        const vec3 e1 = vT[1]-vT[0], e2 = vT[2]-vT[0];
        const vec3 h = cross(dir.xyz, e2);
        const float a = dot(e1,h);
        [[flatten]] if (abs(a) <= 0.f) { _valid = false; };
        IFANY (_valid) {
            const vec3 s = -(orig.xyz+vT[0]), q = cross(s, e1);
            const vec3 uvt = vec3(dot(s,h),dot(dir.xyz,q), dot(e2,q))/precIssue(a);
            uv = uvt.xy, T = uvt.z;
            [[flatten]] if (any(lessThan(uv, -SFN.xx)) || (uv.x+uv.y) > (SFO)) { _valid = false; };
            [[flatten]] if ( T >= N_INFINITY || T > cdist || T < (-SFN) ) { _valid = false; };
        }
#else
        // intersect triangle by transform
        // alternate of http://jcgt.org/published/0005/03/03/paper.pd
        const mat3x4 vT = mat3x4(TLOAD(lvtx, tri*3+0), TLOAD(lvtx, tri*3+1), TLOAD(lvtx, tri*3+2));
        const float dz = dot(dir, vT[2]), oz = dot(orig, vT[2]); T = oz/precIssue(dz);
        [[flatten]] if ( T >= N_INFINITY || T > cdist || T < (-SFN) || abs(dz) <= 0.f ) { _valid = false; };
        IFANY (_valid) {
            const vec4 hit = fma(dir,T.xxxx,-orig); uv = vec2(dot(hit,vT[0]), dot(hit,vT[1]));
            [[flatten]] if (any(lessThan(uv, -SFN.xx)) || (uv.x+uv.y) > (SFO)) { _valid = false; };
        }
#endif
    }
    return (_valid ? T : INFINITY);
}
#endif

#endif
#endif
#endif




// single float 32-bit box intersection
// some ideas been used from http://www.cs.utah.edu/~thiago/papers/robustBVH-v2.pdf
// compatible with AMD radeon min3 and max3

bool intersectCubeF32Single(in vec3 orig, in vec3 dr, in bvec4 sgn, in mat3x2 tMinMax, inout vec4 nfe) 
{ nfe = INFINITY.xxxx; // indefined distance

    // calculate intersection
    [[unroll]] for (int i=0;i<3;i++) tMinMax[i] = vec2(fma(tMinMax[i], dr[i].xx, orig[i].xx));
    [[unroll]] for (int i=0;i<3;i++) tMinMax[i] = vec2(min(tMinMax[i].x, tMinMax[i].y), max(tMinMax[i].x, tMinMax[i].y));

    const float 
        tNear = max3_wrap(tMinMax[0].x, tMinMax[1].x, tMinMax[2].x), 
        tFar  = min3_wrap(tMinMax[0].y, tMinMax[1].y, tMinMax[2].y) * InOne;

    // resolve hit
    const bool isCube = tFar>=tNear && tFar >= -SFN && tNear < N_INFINITY;
    [[flatten]] if (isCube) nfe.xz = vec2(tNear, tFar);
    return isCube;
};


// half float 16/32-bit box intersection (claymore dual style)
// some ideas been used from http://www.cs.utah.edu/~thiago/papers/robustBVH-v2.pdf
// made by DevIL research group
// also, optimized for RPM (Rapid Packed Math) https://radeon.com/_downloads/vega-whitepaper-11.6.17.pdf
// compatible with NVidia GPU too

pbvec2_ intersectCubeDual(inout fvec3_ orig, inout fvec3_ dr, in bvec4 sgn, in fvec4_[3] tMinMax, inout vec4 nfe2)
{ nfe2 = INFINITY.xxxx; // indefined distance

    // calculate intersection
    [[unroll]] for (int i=0;i<3;i++) tMinMax[i] = fvec4_(vec4(fma(tMinMax[i],dr[i].xxxx,orig[i].xxxx)));
    [[unroll]] for (int i=0;i<3;i++) tMinMax[i] = fvec4_(min(tMinMax[i].xy, tMinMax[i].zw), max(tMinMax[i].xy, tMinMax[i].zw));

    const fvec2_
        tNear = max3_wrap(tMinMax[0].xy, tMinMax[1].xy, tMinMax[2].xy), 
        tFar  = min3_wrap(tMinMax[0].zw, tMinMax[1].zw, tMinMax[2].zw) * InOne;
    
    // TODO: improve performance and ops
    const bvec2 isCube = and(and(greaterThanEqual(tFar, tNear), greaterThanEqual(tFar, fvec2_(-SFN))), lessThan(tNear, fvec2_(N_INFINITY)));

    nfe2 = mix(nfe2, vec4(tNear, tFar), bvec4(isCube, isCube));
    return binarize(isCube);
};

#endif
