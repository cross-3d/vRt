#ifndef _VERTEX_H
#define _VERTEX_H

#include "../include/mathlib.glsl"


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

    #if (defined(VERTEX_FILLING) || defined(VTX_TRANSPLIT))
    layout ( binding = 3, set = VTX_SET, rgba32f  ) uniform imageBuffer lvtxIn;
    layout ( binding = 5, set = VTX_SET           ) uniform imageBuffer lvtx;
    #else
    layout ( binding = 3, set = VTX_SET           ) readonly uniform imageBuffer lvtxIn;
    layout ( binding = 5, set = VTX_SET, rgba32f  ) readonly uniform imageBuffer lvtx;
    #endif

    //#define TLOAD_I(img,t) imageLoad(img,t)
    #define TLOAD(img,t) imageLoad(img,t)

    layout ( binding = 4, set = VTX_SET, rgba32ui ) uniform uimage2D attrib_texture_out;
    
    layout ( binding = 6, set = VTX_SET           ) uniform usampler2D attrib_texture;
#endif

#if (defined(ENABLE_VSTORAGE_DATA) || defined(BVH_CREATION))
// bvh uniform unified
layout ( binding = 0, set = 1, std430 ) readonly buffer bvhBlockB { 
    mat4x4 transform;
    mat4x4 transformInv;
    mat4x4 projection;
    mat4x4 projectionInv;
    int leafCount, primitiveCount, entryID, primitiveOffset;
} bvhBlock;

#define BVH_ENTRY bvhBlock.entryID
#define BVH_ENTRY_HALF (bvhBlock.entryID>>1)
#endif

// BVH Zone in ray tracing system
#if (defined(ENABLE_VSTORAGE_DATA) && !defined(BVH_CREATION) && !defined(VERTEX_FILLING))
    layout ( binding = 1, set = 1 ) uniform isamplerBuffer bvhMeta;

    #ifdef USE_F32_BVH
    layout ( binding = 2, set = 1, std430 ) readonly buffer bvhBoxesB { mediump vec4 bvhBoxes[][4]; };
    #else
    layout ( binding = 2, set = 1, std430 ) readonly buffer bvhBoxesB { uvec2 bvhBoxes[][4]; }; 
    #endif
#endif






//#define _SWIZV wzx
#define _SWIZV xyz

const int WARPED_WIDTH = 2048;
const ivec2 mit[3] = {ivec2(0,1), ivec2(1,1), ivec2(1,0)};

ivec2 mosaicIdc(in ivec2 mosaicCoord, const uint idc) {
    mosaicCoord += mit[idc];
#ifdef VERTEX_FILLING
    mosaicCoord.x %= int(imageSize(attrib_texture_out).x);
#endif
    return mosaicCoord;
}

ivec2 gatherMosaic(in ivec2 uniformCoord) {
    return ivec2(uniformCoord.x * 3 + uniformCoord.y % 3, uniformCoord.y);
}

//vec4 fetchMosaic(in sampler2D vertices, in ivec2 mosaicCoord, const uint idc) {
    //return texelFetch(vertices, mosaicCoord + mit[idc], 0);
    //return textureLod(vertices, (vec2(mosaicIdc(mosaicCoord, idc)) + 0.49999f) / textureSize(vertices, 0), 0); // supper native warping
//}

ivec2 getUniformCoord(in int indice) {
    return ivec2(indice % WARPED_WIDTH, indice / WARPED_WIDTH);
}


const mat3 uvwMap = mat3(vec3(1.f,0.f,0.f),vec3(0.f,1.f,0.f),vec3(0.f,0.f,1.f));

#ifndef VERTEX_FILLING
#ifndef BVH_CREATION
#ifdef ENABLE_VSTORAGE_DATA

#ifndef USE_FAST_INTERSECTION
float intersectTriangle(const vec3 orig, const mat3 M, const int axis, const int tri, inout vec2 UV, in bool valid) {
    float T = INFINITY;
    IFANY (valid) {
        // gather patterns
        const int itri = tri*3;//tri*9;
        const mat3 ABC = transpose(M*mat3(
            TLOAD(lvtx, itri+0).xyz-orig,
            TLOAD(lvtx, itri+1).xyz-orig,
            TLOAD(lvtx, itri+2).xyz-orig
        ));

        // watertight triangle intersection (our, GPU-GLSL adapted version)
        // http://jcgt.org/published/0002/01/05/paper.pdf
        vec3 UVW_ = uvwMap[axis] * inverse(ABC);
        IFANY (valid = valid && (all(greaterThan(UVW_, 0.f.xxx)) || all(lessThan(UVW_, 0.f.xxx)))) {
            UVW_ /= precIssue(dot(UVW_, vec3(1)));
            UV = vec2(UVW_.yz), UVW_ *= ABC; // calculate axis distances
            T = mix(mix(UVW_.z, UVW_.y, axis == 1), UVW_.x, axis == 0);
            T = mix(INFINITY, T, (T >= -(1e-5f)) && valid);
        }
    }
    return T;
}
#endif

#ifdef USE_FAST_INTERSECTION
float intersectTriangle(const vec3 orig, const vec3 dir, const int tri, inout vec2 uv, in bool _valid) {
    const int itri = tri*3;//tri*9;
    //const mat3 vT = transpose(mat3(
    const mat3 vT = mat3(
        TLOAD(lvtxIn, itri+0).xyz,
        TLOAD(lvtxIn, itri+1).xyz,
        TLOAD(lvtxIn, itri+2).xyz
    );//));
    const vec3 e1 = vT[1]-vT[0], e2 = vT[2]-vT[0];
    const vec3 h = cross(dir, e2);
    const float a = dot(e1,h);

#ifdef BACKFACE_CULLING
    if (a < 0.f) { _valid = false; }
//    if (a < 1e-5f) { _valid = false; }
//#else
//    if (abs(a) < 1e-5f) { _valid = false; }
#endif

    const float f = 1.f/precIssue(a);
    const vec3 s = orig - vT[0], q = cross(s, e1);
    uv = f * vec2(dot(s,h),dot(dir,q));

    if (uv.x <= -1e-6 || uv.y <= -1e-6 || (uv.x+uv.y) >= (1.f+1e-6)) { _valid = false; }

    float T = f * dot(e2,q);
    if (T >= INFINITY || T < 0.f) { _valid = false; } 
    if (!_valid) T = INFINITY;
    return T;
}
#endif

#endif
#endif
#endif





const int _BVH_WIDTH = 2048;

#ifdef ENABLE_VSTORAGE_DATA
#ifdef ENABLE_VERTEX_INTERPOLATOR
// barycentric map (for corrections tangents in POM)
void interpolateMeshData(inout VtHitData ht) {
    const int tri = floatBitsToInt(ht.uvt.w)-1;
    const vec3 vs = vec3(1.0f - ht.uvt.x - ht.uvt.y, ht.uvt.xy);
    const vec2 sz = 1.f.xx / textureSize(attrib_texture, 0);
    const bool validInterpolant = ht.attribID > 0 && vmaterials[tri] == ht.materialID;
    IFANY (validInterpolant) {
        [[unroll]]
        for (int i=0;i<ATTRIB_EXTENT;i++) {
            const vec2 trig = fma(vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+i))), sz, sz*0.5f);
            imageStore(attributes, makeAttribID(ht.attribID, i), vs * mat4x3(
                SGATHER(attrib_texture, trig, 0)._SWIZV,
                SGATHER(attrib_texture, trig, 1)._SWIZV,
                SGATHER(attrib_texture, trig, 2)._SWIZV,
                SGATHER(attrib_texture, trig, 3)._SWIZV
            ));
        }
    }
}
#endif
#endif


#ifdef VERTEX_FILLING
void storeAttribute(in ivec3 cdata, in vec4 fval) {
    ivec2 ATTRIB_ = gatherMosaic(getUniformCoord(cdata.x*ATTRIB_EXTENT+cdata.y));
    ISTORE(attrib_texture_out, mosaicIdc(ATTRIB_, cdata.z), floatBitsToUint(fval));
}

void storePosition(in ivec2 cdata, in vec4 fval){
    imageStore(lvtxIn, cdata.x*3+cdata.y, fval);
}
#endif


#endif
