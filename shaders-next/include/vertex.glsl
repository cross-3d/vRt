#ifndef _VERTEX_H
#define _VERTEX_H

#include "../include/mathlib.glsl"

//#define VRT_INTERPOLATOR_TEXEL

#ifdef ENABLE_VERTEX_INTERPOLATOR
#ifndef ENABLE_VSTORAGE_DATA
#define ENABLE_VSTORAGE_DATA
#endif
#endif



// Geometry Zone
#if (defined(ENABLE_VSTORAGE_DATA) || defined(BVH_CREATION) || defined(VERTEX_FILLING))
    #ifdef VERTEX_FILLING
    #define VTX_SET 0
    #else
    #define VTX_SET 2
    #endif

    #ifdef VERTEX_FILLING
    //layout ( binding = 0, set = VTX_SET, std430   ) buffer tcounterB { int tcounter[2]; };
    layout ( binding = 1, set = VTX_SET, std430   ) buffer materialsB { int vmaterials[]; };
    layout ( binding = 2, set = VTX_SET, std430   ) buffer bitfieldsB { uint vbitfields[]; };
    #else
    //layout ( binding = 0, set = VTX_SET, std430   ) readonly buffer tcounterB { int tcounter[2]; };
    layout ( binding = 1, set = VTX_SET, std430   ) readonly buffer materialsB { int vmaterials[]; };
    layout ( binding = 2, set = VTX_SET, std430   ) readonly buffer bitfieldsB { int vbitfields[]; };
    #endif

    #ifdef VERTEX_FILLING
    layout ( binding = 3, set = VTX_SET, rgba32f ) coherent uniform imageBuffer lvtxIn;
    #else
    layout ( binding = 3, set = VTX_SET, rgba32f ) readonly uniform imageBuffer lvtxIn;
    #endif

    #if (defined(LEAF_GEN) || defined(VERTEX_FILLING))
    layout ( binding = 5, set = VTX_SET, rgba32f ) coherent uniform imageBuffer lvtx;
    layout ( binding = 7, set = VTX_SET, rgba32f ) coherent uniform imageBuffer lnrm;
    #else
    layout ( binding = 5, set = VTX_SET, rgba32f ) readonly uniform imageBuffer lvtx;
    layout ( binding = 7, set = VTX_SET, rgba32f ) readonly uniform imageBuffer lnrm;
    #endif

    #define TLOAD(img,t) imageLoad(img,t)
    layout ( binding = 4, set = VTX_SET, rgba32f ) uniform highp image2D attrib_texture_out;
    layout ( binding = 6, set = VTX_SET          ) uniform highp sampler2D attrib_texture;

#endif

#if (defined(ENABLE_VSTORAGE_DATA) || defined(BVH_CREATION))
// bvh uniform unified
layout ( binding = 0, set = 1, std430 ) readonly restrict buffer bvhBlockB { 
    mat4x4  transform,  transformInv;
    mat4x4 projection, projectionInv;
    int leafCount, primitiveCount, entryID, primitiveOffset;
    vec4 sceneMin, sceneMax;
} bvhBlock;

vec4 uniteBox(in vec4 glb) { return fma((glb - vec4(bvhBlock.sceneMin.xyz, 0.f)) / vec4(precIssue(bvhBlock.sceneMax.xyz - bvhBlock.sceneMin.xyz), 1.f), vec4( 2.f.xxx,  1.f), vec4(-1.f.xxx, 0.f)); };



#define BVH_ENTRY bvhBlock.entryID
#define BVH_ENTRY_HALF (bvhBlock.entryID>>1)
#endif

// BVH Zone in ray tracing system
#if (defined(ENABLE_VSTORAGE_DATA) && !defined(BVH_CREATION) && !defined(VERTEX_FILLING))
struct NTYPE_ {
#ifdef USE_F32_BVH
     vec4 cbox[3];
    ivec4 meta;
#else
    f16vec4 cbox[3];
      ivec4 meta;
#endif
};

layout ( binding = 2, set = 1, std430 ) readonly coherent buffer bvhBoxesB { NTYPE_ bvhNodes[]; };
#endif



#ifdef VRT_INTERPOLATOR_TEXEL
//const int WARPED_WIDTH = 4096;
const int WARPED_WIDTH = 6144;
//const ivec2 mit[4] = { ivec2(1,0), ivec2(0,0), ivec2(0,1),  ivec2(1,1) };
const ivec2 mit[4] = { ivec2(0,0), ivec2(1,0), ivec2(0,1),  ivec2(1,1) };
ivec2 gatherMosaic(in ivec2 uniformCoord) {
    //return ivec2((uniformCoord.x * 3) + (uniformCoord.y % 3), uniformCoord.y);
    return ivec2(uniformCoord)<<1;
}

#else

const int WARPED_WIDTH = 4096;
const ivec2 mit[4] = {ivec2(0,1), ivec2(1,1), ivec2(1,0), ivec2(0,0)};
ivec2 gatherMosaic(in ivec2 uniformCoord) {
    return ivec2((uniformCoord.x * 3) + (uniformCoord.y % 3), uniformCoord.y);
}

#endif



ivec2 mosaicIdc(in ivec2 mosaicCoord, in uint idc) {
    mosaicCoord += mit[idc];
#ifdef VERTEX_FILLING
    mosaicCoord.x %= int(imageSize(attrib_texture_out).x);
#endif
    return mosaicCoord;
}

ivec2 getUniformCoord(in int indice) {
    return ivec2(indice % WARPED_WIDTH, indice / WARPED_WIDTH);
}


const mat3 uvwMap = mat3(vec3(1.f,0.f,0.f),vec3(0.f,1.f,0.f),vec3(0.f,0.f,1.f));

#ifndef VERTEX_FILLING
#ifndef BVH_CREATION
#ifdef ENABLE_VSTORAGE_DATA
 
#ifndef VRT_USE_FAST_INTERSECTION
float intersectTriangle(in vec4 orig, in mat3 M, in int axis, in int tri, inout vec2 UV, in bool _valid) {
    float T = INFINITY;
    IFANY (_valid) {
        const mat3 ABC = mat3(TLOAD(lvtx, tri*3+0).xyz+orig.xxx, TLOAD(lvtx, tri*3+1).xyz+orig.yyy, TLOAD(lvtx, tri*3+2).xyz+orig.zzz)*M;

        // watertight triangle intersection (our, GPU-GLSL adapted version)
        // http://jcgt.org/published/0002/01/05/paper.pdf
        vec3 UVW_ = uvwMap[axis] * inverse(ABC);
        IFANY ((all(greaterThan(UVW_, 0.f.xxx)) || all(lessThan(UVW_, 0.f.xxx))) && _valid) {
            UVW_ /= precIssue(dot(UVW_, vec3(1)));
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
        [[flatten]] if ( T >= N_INFINITY || T > cdist || T < (-SFN) ) { _valid = false; };
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








#ifdef ENABLE_VSTORAGE_DATA
#ifdef ENABLE_VERTEX_INTERPOLATOR
#define _SWIZV xyz
// barycentric map (for corrections tangents in POM)
void interpolateMeshData(inout VtHitData ht, in int tri) {
    const vec3 vs = vec3(1.0f - ht.uvt.x - ht.uvt.y, ht.uvt.xy);
    const vec2 sz = 1.f.xx / textureSize(attrib_texture, 0);
    [[flatten]] if (ht.attribID > 0) {
        //[[unroll]] for (int i=0;i<ATTRIB_EXTENT;i++) {
        [[dont_unroll]] for (int i=0;i<ATTRIB_EXTENT;i++) {
#ifdef VRT_INTERPOLATOR_TEXEL
            const vec2 trig = (vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+i))) + vs.yz + 0.5f) * sz;
            ISTORE(attributes, makeAttribID(ht.attribID, i), textureHQ(attrib_texture, trig, 0));
#else
            const vec2 trig = (vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+i))) + 0.5f) * sz;
            ISTORE(attributes, makeAttribID(ht.attribID, i), vs * mat4x3(
                SGATHER(attrib_texture, trig, 0)._SWIZV,
                SGATHER(attrib_texture, trig, 1)._SWIZV,
                SGATHER(attrib_texture, trig, 2)._SWIZV,
                SGATHER(attrib_texture, trig, 3)._SWIZV
            ));
#endif
        }
    }
}
#endif
#endif


#ifdef VERTEX_FILLING
void storeAttribute(in ivec3 cdata, in vec4 fval) {
    const ivec2 ATTRIB_ = gatherMosaic(getUniformCoord(cdata.x*ATTRIB_EXTENT+cdata.y));
    [[flatten]] if (cdata.z < 3) {
        ISTORE(attrib_texture_out, mosaicIdc(ATTRIB_,cdata.z), (fval));
    } else {
#ifdef VRT_INTERPOLATOR_TEXEL
        const vec3 vs = vec3(-1.f,1.f,1.f);
        ISTORE(attrib_texture_out, mosaicIdc(ATTRIB_,3), mat3x4(
            TLOAD(attrib_texture_out, mosaicIdc(ATTRIB_,0)),
            TLOAD(attrib_texture_out, mosaicIdc(ATTRIB_,1)),
            TLOAD(attrib_texture_out, mosaicIdc(ATTRIB_,2))
        ) * vs);
#endif
    }
}

void storePosition(in ivec2 cdata, in vec4 fval) {
    ISTORE(lvtxIn, cdata.x*3+cdata.y, fval);
}
#endif



// single float 32-bit box intersection
// some ideas been used from http://www.cs.utah.edu/~thiago/papers/robustBVH-v2.pdf
// compatible with AMD radeon min3 and max3

bool intersectCubeF32Single(in vec3 origin, in vec3 dr, in bvec3 sgn, in mat3x2 tMinMax, inout vec4 nfe) {
    nfe = INFINITY.xxxx;
    tMinMax = mat3x2(fma(tMinMax[0], dr.xx, origin.xx), fma(tMinMax[1], dr.yy, origin.yy), fma(tMinMax[2], dr.zz, origin.zz));
    [[unroll]] for (int i=0;i<3;i++) tMinMax[i] = tMinMax[i].x <= tMinMax[i].y ? tMinMax[i] : tMinMax[i].yx;
    //[[unroll]] for (int i=0;i<3;i++) tMinMax[i] = vec2(min(tMinMax[i].x, tMinMax[i].y), max(tMinMax[i].x, tMinMax[i].y));

    const float 
        tNear = max3_wrap(tMinMax[0].x, tMinMax[1].x, tMinMax[2].x), 
        tFar  = min3_wrap(tMinMax[0].y, tMinMax[1].y, tMinMax[2].y) * InOne;

    // resolve hit
    const bool isCube = tFar>=tNear && tFar >= -SFN && tNear < N_INFINITY;
    nfe.xz = mix(nfe.xz, vec2(tNear, tFar), isCube.xx);
    return isCube;
};


// half float 16/32-bit box intersection (claymore dual style)
// some ideas been used from http://www.cs.utah.edu/~thiago/papers/robustBVH-v2.pdf
// made by DevIL research group
// also, optimized for RPM (Rapid Packed Math) https://radeon.com/_downloads/vega-whitepaper-11.6.17.pdf
// compatible with NVidia GPU too

#if (!defined(AMD_F16_BVH) && !defined(USE_F32_BVH)) // identify as mediump
lowp bvec2_ intersectCubeDual(in mediump fvec3_ origin, in mediump fvec3_ dr, in bvec3 sgn, in highp fmat3x4_ tMinMax, inout vec4 nfe2)
#else
lowp bvec2_ intersectCubeDual(in fvec3_ origin, in fvec3_ dr, in bvec3 sgn, in fmat3x4_ tMinMax, inout vec4 nfe2)
#endif
{
    nfe2 = INFINITY.xxxx;
    tMinMax = fmat3x4_(fma(tMinMax[0], dr.xxxx, origin.xxxx), fma(tMinMax[1], dr.yyyy, origin.yyyy), fma(tMinMax[2], dr.zzzz, origin.zzzz));
    //[[unroll]] for (int i=0;i<3;i++) tMinMax[i] = vec4(mix(tMinMax[i].zwxy, tMinMax[i], lessThanEqual(tMinMax[i].xy, tMinMax[i].zw).xyxy));
    [[unroll]] for (int i=0;i<3;i++) tMinMax[i] = fvec4_(min(tMinMax[i].xy, tMinMax[i].zw), max(tMinMax[i].xy, tMinMax[i].zw));

    const 
#if (!defined(AMD_F16_BVH) && !defined(USE_F32_BVH)) // identify as mediump
    mediump
#endif
    fvec2_
        tNear = max3_wrap(tMinMax[0].xy, tMinMax[1].xy, tMinMax[2].xy), 
        tFar  = min3_wrap(tMinMax[0].zw, tMinMax[1].zw, tMinMax[2].zw) * InOne;
        
    const bvec2_ isCube = 
        bvec2_(greaterThanEqual(tFar, tNear)) & 
        bvec2_(greaterThanEqual(tFar, fvec2_(-SFN))) & 
        bvec2_(lessThan(tNear, fvec2_(N_INFINITY)));

    nfe2 = mix(nfe2, vec4(tNear, tFar), bvec4(isCube, isCube));
    return isCube;
};

#endif
