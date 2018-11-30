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
        #define VTX_SET 2
    #endif

    #ifdef VERTEX_FILLING
    layout ( binding = 2, set = VTX_SET, align_ssbo ) coherent buffer bitfieldsB { uint vbitfields[]; };
    #else
    layout ( binding = 2, set = VTX_SET, align_ssbo ) readonly buffer bitfieldsB { uint vbitfields[]; };
    #endif

    #if (defined(LEAF_GEN) || defined(VERTEX_FILLING))
    layout ( binding = 5, set = VTX_SET, rgba32f ) coherent uniform  imageBuffer lvtx;
    layout ( binding = 7, set = VTX_SET, rgba32f ) coherent uniform  imageBuffer lnrm;
    layout ( binding = 8, set = VTX_SET, r32ui   )   coherent uniform uimageBuffer indx;
    #else
    layout ( binding = 5, set = VTX_SET, rgba32f ) readonly uniform  imageBuffer lvtx;
    layout ( binding = 7, set = VTX_SET, rgba32f ) readonly uniform  imageBuffer lnrm;
    layout ( binding = 8, set = VTX_SET, r32ui   )   readonly uniform uimageBuffer indx;
    #endif

#endif


// task level traverse data 
struct BvhBlockT {
    int entryID, leafCount; // leafCount reserved for ESC versions
    int primitiveCount, primitiveOffset; // first for default triangle limit, second for correct vertex data block ( because assembled set should have shared buffer )
    mat3x4 transform;//, transformInv; // we resolved to save that row 
    vec4 sceneMin, sceneMax;
};

// 
struct BvhInstanceT {
    mat3x4 transformIn; // row of traversion correction, combined with transforming to instance space 
    int bvhBlockID, bvhDataID, r1, r2;
};

#if (defined(USE_F32_BVH) || defined(USE_F16_BVH)) && !defined(EXPERIMENTAL_UNORM16_BVH)
    #define nbox_t fvec4_[3]
#else
    #define nbox_t uvec2[3]
#endif


// required 64-byte per full node 
struct BTYPE_ {
    nbox_t cbox;
#if (defined(USE_F16_BVH) || defined(EXPERIMENTAL_UNORM16_BVH) || !defined(USE_F32_BVH))
    uvec2 spacing[3]; // when using 16-bit data, need have data space ()
#endif
    ivec4 meta;
};


#ifndef VERTEX_FILLING
// Block of main BVH structure (for bottom levels will not required)
layout ( binding = 0, set = 1, align_ssbo ) readonly buffer bvhBlockB { BvhBlockT data[]; } bvhBlockState_[]; // bvhBlock of main structure 


// Accessible blocks and instances for top levels, or task accessing (required shared buffers)
#ifdef EXPERIMENTAL_INSTANCING_SUPPORT
#ifdef BVH_CREATION
layout ( binding = 2, set = 1, align_ssbo ) buffer BvhInstanceB { BvhInstanceT bvhInstance_[]; };
//layout ( binding = 3, set = 1, align_ssbo ) buffer bvhBlockInB { BvhBlockT bvhBlockIn_[]; };
layout ( binding = 4, set = 1, align_ssbo ) buffer BvhTransformB { mat3x4 transformData_[]; };
#else
layout ( binding = 2, set = 1, align_ssbo ) readonly buffer BvhInstanceB { BvhInstanceT bvhInstance_[]; };
//layout ( binding = 3, set = 1, align_ssbo ) readonly buffer bvhBlockInB { BvhBlockT bvhBlockIn_[]; };
layout ( binding = 4, set = 1, align_ssbo ) readonly buffer BvhTransformB { mat3x4 transformData_[]; };
#endif
#endif


#ifdef BVH_CREATION
layout ( binding = 1, set = 1, align_ssbo )          buffer bvhBoxesB { BTYPE_ bvhNodes_[]; } bInstances[];
#else
layout ( binding = 1, set = 1, align_ssbo ) readonly buffer bvhBoxesB { BTYPE_ bvhNodes_[]; } bInstances[];
#endif
#endif



const uint BVH_STATE_TOP = 0, BVH_STATE_BOTTOM = 1;
const mat3 uvwMap = mat3(vec3(1.f,0.f,0.f),vec3(0.f,1.f,0.f),vec3(0.f,0.f,1.f));
const float SFNa = SFN *1.f, SFOa = SFNa+1.f;
const uint lastDataID = 0u;

// int topLevelEntry = 0; // should be scalar value
 int topLevelEntry = 0; // should be scalar 
uint currentState = BVH_STATE_TOP;
 int INSTANCE_ID = -1, LAST_INSTANCE = -1, RAY_ID = -1, MAX_ELEMENTS = 0;

// instanced BVH node
#define bvhInstance bvhInstance_[INSTANCE_ID]
#define instanceTransform transformData_[INSTANCE_ID]
#define bvhBlockTop bvhBlockState_[0].data[0]
#define bvhBlockIn  bvhBlockState_[NonUniform(lastDataID)].data[currentState==BVH_STATE_TOP?0:bvhInstance.bvhBlockID]


//#define bvhNodes bInstances[NonUniform(uint(1+((currentState==BVH_STATE_TOP)?-1:bvhInstance.bvhDataID)))].bvhNodes_
  #define bvhNodes bInstances[NonUniform(lastDataID)].bvhNodes_
//#define bvhNodes bInstances[NonUniform(0u)].bvhNodes_

// instanced BVH entry
#define BVH_ENTRY bvhBlockIn.entryID
#define BVH_ENTRY_HALF (BVH_ENTRY>>1)




#ifndef VERTEX_FILLING
//vec4 uniteBox(in vec4 glb) { return fma((glb - vec4(bvhBlock.sceneMin.xyz, 0.f)) / vec4((bvhBlock.sceneMax.xyz - bvhBlock.sceneMin.xyz), 1.f), vec4( 2.f.xxx,  1.f), vec4(-1.f.xxx, 0.f)); };
vec4 uniteBox   (in vec4 glb) { return point4(fma((glb - bvhBlockIn .sceneMin) / (bvhBlockIn .sceneMax - bvhBlockIn .sceneMin), 2.f.xxxx, -1.f.xxxx), glb.w); };
vec4 uniteBoxTop(in vec4 glb) { return point4(fma((glb - bvhBlockTop.sceneMin) / (bvhBlockTop.sceneMax - bvhBlockTop.sceneMin), 2.f.xxxx, -1.f.xxxx), glb.w); };
#endif



#ifndef VERTEX_FILLING
#ifndef BVH_CREATION
#ifdef ENABLE_VSTORAGE_DATA

#ifndef VRT_USE_FAST_INTERSECTION
float intersectTriangle(in vec4 orig, in mat3 M, in int axis, in int tri, inout vec2 UV, inout bool _valid) {
    float T = INFINITY;
    //IFANY (_valid) {
    [[flatten]] if (_valid) {
        const mat3 ABC = mat3((TLOAD(lvtx, tri*3+0)+orig.x).xyz, (TLOAD(lvtx, tri*3+1)+orig.y).xyz, (TLOAD(lvtx, tri*3+2)+orig.z).xyz)*M;

        // watertight triangle intersection (our, GPU-GLSL adapted version)
        // http://jcgt.org/published/0002/01/05/paper.pdf
        vec3 UVW_ = uvwMap[axis] * inverse(ABC);
        //IFANY ((all(greaterThan(UVW_, 0.f.xxx)) || all(lessThan(UVW_, 0.f.xxx))) && _valid) {
        [[flatten]] if ((all(greaterThan(UVW_, 0.f.xxx)) || all(lessThan(UVW_, 0.f.xxx))) && _valid) {
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
float intersectTriangle(in vec4 orig, in vec4 dir, in int tri, inout vec2 uv, inout bool _valid) {
    float T = INFINITY;
    [[flatten]] if (_valid) {
#ifdef VTX_USE_MOLLER_TRUMBORE
        // classic intersection (Möller–Trumbore)
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        //const mat3 vT = mat3(TLOAD(lvtx, tri*3+0).xyz, TLOAD(lvtx, tri*3+1).xyz, TLOAD(lvtx, tri*3+2).xyz);
        const vec3 v0 = TLOAD(lvtx, tri*3+0).xyz, e1 = TLOAD(lvtx, tri*3+1).xyz-v0, e2 = TLOAD(lvtx, tri*3+2).xyz-v0;
        const vec3 h = cross(dir.xyz, e2); const float dz = dot(e1,h);
        const vec3 s = -(orig.xyz+v0), q = cross(s, e1), uvt = vec3(dot(s,h),dot(dir.xyz,q),dot(e2,q))/(dz);
        uv = uvt.xy, T = uvt.z;
#else
        // intersect triangle by transform
        // alternate of http://jcgt.org/published/0005/03/03/paper.pd
        const mat3x4 vT = mat3x4(TLOAD(lvtx, tri*3+0), TLOAD(lvtx, tri*3+1), TLOAD(lvtx, tri*3+2));
        const float dz = dot(dir, vT[2]), oz = dot(orig, vT[2]); T = oz/(dz);
        const vec4 hit = fma(dir,T.xxxx,-orig); uv = vec2(dot(hit,vT[0]), dot(hit,vT[1]));
#endif
        [[flatten]] if (T >= N_INFINITY || abs(dz) <= 0.f || any(lessThanEqual(vec4(SFO-uv.x-uv.y, uv, T), SFN.xxxx))) { _valid = false; };
    };
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
{
    tMinMax = mat3x2(
        fma(tMinMax[0], dr.xx, orig.xx), 
        fma(tMinMax[1], dr.yy, orig.yy), 
        fma(tMinMax[2], dr.zz, orig.zz)
    );
    tMinMax = mat3x2(
        vec2(min(tMinMax[0].x, tMinMax[0].y), max(tMinMax[0].x, tMinMax[0].y)), 
        vec2(min(tMinMax[1].x, tMinMax[1].y), max(tMinMax[1].x, tMinMax[1].y)), 
        vec2(min(tMinMax[2].x, tMinMax[2].y), max(tMinMax[2].x, tMinMax[2].y))
    );

    const float 
        tNear = max3_wrap(tMinMax[0].x, tMinMax[1].x, tMinMax[2].x), 
        tFar  = min3_wrap(tMinMax[0].y, tMinMax[1].y, tMinMax[2].y) * InOne;

    // resolve hit
    const bool isCube = tFar>=tNear && tFar >= -SFN && tNear < N_INFINITY;
    nfe.xz = isCube ? vec2(tNear, tFar) : INFINITY.xx;
    return isCube;
};


// half float 16/32-bit box intersection (claymore dual style)
// some ideas been used from http://www.cs.utah.edu/~thiago/papers/robustBVH-v2.pdf
// made by DevIL research group
// also, optimized for RPM (Rapid Packed Math) https://radeon.com/_downloads/vega-whitepaper-11.6.17.pdf
// compatible with NVidia GPU too

#ifdef EXPERIMENTAL_UNORM16_BVH
    #define fcvt4_ unpackSnorm4x16
#else
    #define fcvt4_ fvec4_
#endif

#ifndef VERTEX_FILLING
pbvec2_ intersectCubeDual(inout fvec3_ orig, inout fvec3_ dr, in bvec4 sgn, in nbox_t cbox, inout vec4 nfe2)
{
    // calculate intersection
    fvec4_[3] tMinMax = { 
        fma(fcvt4_(cbox[0]),dr.xxxx,orig.xxxx), 
        fma(fcvt4_(cbox[1]),dr.yyyy,orig.yyyy), 
        fma(fcvt4_(cbox[2]),dr.zzzz,orig.zzzz)
    };
    tMinMax = fvec4_[3](
        fvec4_(min(tMinMax[0].xy,tMinMax[0].zw),max(tMinMax[0].xy,tMinMax[0].zw)),
        fvec4_(min(tMinMax[1].xy,tMinMax[1].zw),max(tMinMax[1].xy,tMinMax[1].zw)),
        fvec4_(min(tMinMax[2].xy,tMinMax[2].zw),max(tMinMax[2].xy,tMinMax[2].zw))
    );

    const fvec2_
        tNear = max3_wrap(tMinMax[0].xy, tMinMax[1].xy, tMinMax[2].xy), 
        tFar  = min3_wrap(tMinMax[0].zw, tMinMax[1].zw, tMinMax[2].zw) * InOne;
    
    // TODO: improve performance and ops
    const bvec2 isCube = and(and(greaterThanEqual(tFar, tNear), greaterThan(tFar, fvec2_(SFN))), lessThan(tNear, fvec2_(N_INFINITY)));
    nfe2 = mix(INFINITY.xxxx, vec4(tNear, tFar), bvec4(isCube, isCube));
    return binarize(isCube);
};
#endif

#endif
