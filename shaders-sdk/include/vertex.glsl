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
    layout ( binding = 2, set = VTX_SET, align_ssbo ) coherent buffer bitfieldsB { uint32_t vbitfields[]; };
    #else
    layout ( binding = 2, set = VTX_SET, align_ssbo ) readonly buffer bitfieldsB { uint32_t vbitfields[]; };
    #endif

    #if (defined(LEAF_GEN) || defined(VERTEX_FILLING))
    layout ( binding = 5, set = VTX_SET, align_ssbo ) coherent buffer VTX_BUFFER { f32vec4 data[]; } lvtx[];
    layout ( binding = 7, set = VTX_SET, align_ssbo ) coherent buffer NRM_BUFFER { f32vec4 data[]; } lnrm[];

    #else
    //layout ( binding = 5, set = VTX_SET, align_ssbo ) readonly buffer VTX_BUFFER { f32vec4 data[]; } lvtx[];
    layout ( binding = 7, set = VTX_SET, align_ssbo ) readonly buffer NRM_BUFFER { f32vec4 data[]; } lnrm[];
    layout ( binding = 8, set = VTX_SET ) uniform samplerBuffer lvtxT[];

    // fetchers 
    const int vtd = 4; // vertex input striding 
    //f32vec3 v3fetch(in samplerBuffer lvtxT, in int t) { return vec3(texelFetch(lvtxT,t*vtd+0).x,texelFetch(lvtxT,t*vtd+1).x,texelFetch(lvtxT,t*vtd+2).x); };
    //f32vec4 v4fetch(in samplerBuffer lvtxT, in int t) { return vec4(texelFetch(lvtxT,t*vtd+0).x,texelFetch(lvtxT,t*vtd+1).x,texelFetch(lvtxT,t*vtd+2).x,texelFetch(lvtxT,t*vtd+3).x); };
    f32vec3 v3fetch(in samplerBuffer lvtxT, in int t) { return texelFetch(lvtxT,t).xyz; };
    f32vec4 v4fetch(in samplerBuffer lvtxT, in int t) { return texelFetch(lvtxT,t); };
    #endif

#endif


// task level traverse data 
struct BvhBlockT {
    f32mat3x4 transform;
    int32_t elementsOffset, elementsCount, entryID; uint32_t bitfield;
    f32vec4 sceneMin, sceneMax;
};

// 
struct BvhInstanceT {
    f32mat3x4 transformIn;
    int32_t bvhBlockID, bvhDataID, r1, r2;
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
    u32vec2 spacing[3]; // when using 16-bit data, need have data space ()
#endif
    i32vec4 meta;
};


#ifndef VERTEX_FILLING
// Block of main BVH structure (for bottom levels will not required)
layout ( binding = 0, set = 1, align_ssbo ) readonly buffer bvhBlockB { BvhBlockT data[]; } bvhBlockState_[]; // bvhBlock of main structure 


// Accessible blocks and instances for top levels, or task accessing (required shared buffers)
#ifdef EXPERIMENTAL_INSTANCING_SUPPORT
#ifdef BVH_CREATION
layout ( binding = 2, set = 1, align_ssbo ) coherent buffer BvhInstanceB { BvhInstanceT bvhInstance_[]; };
layout ( binding = 4, set = 1, align_ssbo ) coherent buffer BvhTransformB { mat3x4 transformData_[]; };
#else
layout ( binding = 2, set = 1, align_ssbo ) readonly buffer BvhInstanceB { BvhInstanceT bvhInstance_[]; };
layout ( binding = 4, set = 1, align_ssbo ) readonly buffer BvhTransformB { mat3x4 transformData_[]; };
#endif
#endif


#ifdef BVH_CREATION
layout ( binding = 1, set = 1, align_ssbo ) coherent buffer bvhBoxesB { BTYPE_ bvhNodes_[]; } bInstances[];
#else
layout ( binding = 1, set = 1, align_ssbo ) readonly buffer bvhBoxesB { BTYPE_ bvhNodes_[]; } bInstances[];
#endif
#endif



const uint BVH_STATE_TOP = 0, BVH_STATE_BOTTOM = 1;
const mat3 uvwMap = mat3(vec3(1.f,0.f,0.f),vec3(0.f,1.f,0.f),vec3(0.f,0.f,1.f));
const float SFNa = SFN *1.f, SFOa = SFNa+1.f;
const uint lastDataID = 0u;

 int topLevelEntry = -1; uint CACHE_ID = 0; uint currentState = BVH_STATE_TOP;
 int INSTANCE_ID = -1, LAST_INSTANCE = -1, RAY_ID = -1, MAX_ELEMENTS = 0;

//#define topLevelEntry varset[Wave_Idx][0]
//#define CACHE_ID varset[Wave_Idx][1]

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

#ifndef USE_FAST_INTERSECTION
struct wt_input_t { mat3 M; int axis; };
bool intersectTriangle(inout vec4 orig, inout wt_input_t dir, in int tri, inout vec3 UVT, inout bool _valid) {
    [[flatten]] if (_valid) 
    {
        const mat3 ABC = mat3(v3fetch(lvtxT[0],tri*3+0)+orig.x, v3fetch(lvtxT[0],tri*3+1)+orig.y, v3fetch(lvtxT[0],tri*3+2)+orig.z)*dir.M;

        // watertight triangle intersection (our, GPU-GLSL adapted version)
        // http://jcgt.org/published/0002/01/05/paper.pdf
        vec3 UVW = uvwMap[dir.axis] * inverse(ABC); UVW /= dot(UVW,1.f.xxx);
        [[flatten]] if (all(greaterThan(UVW,0.f.xxx)) || all(lessThan(UVW,0.f.xxx))) {
            UVT = vec3(UVW.yz,dot(UVW,ABC[dir.axis])); // calculate axis distances
            [[flatten]] if ( UVT.z < (-SFN) || UVT.z >= N_INFINITY ) _valid = false;
        } else { _valid = false; };
    };
    [[flatten]] if (!_valid) UVT.z = INFINITY;
    return _valid;
};
#else
bool intersectTriangle(inout vec4 orig, inout vec4 dir, in int tri, inout vec3 UVT, inout bool _valid) {
    [[flatten]] if (_valid) 
    {
#ifdef USE_MOLLER_TRUMBORE
        // classic intersection (Möller–Trumbore)
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        const vec4 s = v4fetch(lvtxT[0],tri*3+0)-orig, e1 = v4fetch(lvtxT[0],tri*3+1), e2 = v4fetch(lvtxT[0],tri*3+2);
        const vec4 h = crossp4(dir,e2), q = crossp4(s,e1); const float dz = dot(e1,h);
        UVT = vec3(dot(s,h),dot(dir,q),dot(e2,q))/dz;
#else
        // intersect triangle by transform
        // alternate of http://jcgt.org/published/0005/03/03/paper.pd
        const mat3x4 vT = mat3x4(v4fetch(lvtxT[0], tri*3+0), v4fetch(lvtxT[0], tri*3+1), v4fetch(lvtxT[0], tri*3+2));
        const float  dz = dot(dir,vT[2]),oz = dot(orig,vT[2]); UVT.z = oz/dz;
        const vec4  hit = fma(dir, UVT.zzzz, -orig); UVT.xy = vec2(dot(hit,vT[0]), dot(hit,vT[1]));
#endif
        [[flatten]] if (UVT.z >= N_INFINITY || abs(dz) <= 0.f || any(lessThanEqual(vec4(SFO-UVT.x-UVT.y,UVT),SFN.xxxx))) { _valid = false; };
    };
    [[flatten]] if (!_valid) UVT.z = INFINITY;
    return _valid;
};
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
bvec2 intersectCubeDual(inout fvec3_ orig, inout fvec3_ dr, in bvec4 sgn, in nbox_t cbox, inout vec4 nfe2)
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
    return isCube;
};
#endif

#endif
