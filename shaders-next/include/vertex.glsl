#ifndef _VERTEX_H
#define _VERTEX_H

//#define BACKFACE_CULLING

#include "../include/mathlib.glsl"

// enable this data for interpolation meshes
#ifdef ENABLE_VERTEX_INTERPOLATOR
#ifndef ENABLE_VSTORAGE_DATA
#define ENABLE_VSTORAGE_DATA
#endif
#endif

// for geometry accumulators
#ifdef VERTEX_FILLING
    layout ( std430, binding = 0, set = 0 ) buffer tcounterB { int tcounter[2]; };
    layout ( std430, binding = 1, set = 0 ) buffer materialsB { int materials[]; };
    layout ( std430, binding = 2, set = 0 ) buffer vordersB { int vorders[]; };
    //layout ( std430, binding = 3, set = 0 ) buffer lvtxB { float lvtx[]; };
    layout ( binding = 3, set = 0, rgba32f ) uniform imageBuffer lvtx;
    layout ( rgba32ui, binding = 4, set = 0 ) uniform uimage2D attrib_texture_out;
#else
    layout ( std430, binding = 1, set = 1 ) readonly buffer materialsB { int materials[]; };

    #ifdef ENABLE_VERTEX_INTERPOLATOR
        layout ( binding = 10, set = 1 ) uniform usampler2D attrib_texture;
        layout ( std430, binding = 2, set = 1 ) readonly buffer vordersB { int vorders[]; };
    #endif

    #ifdef ENABLE_VSTORAGE_DATA
        #ifdef ENABLE_TRAVERSE_DATA
        #ifndef BVH_CREATION
            #ifdef USE_F32_BVH
            layout ( std430, binding = 0, set = 1 ) readonly buffer bvhBoxesB { highp vec4 bvhBoxes[][4]; };
            #else
            layout ( std430, binding = 0, set = 1 ) readonly buffer bvhBoxesB { uvec2 bvhBoxes[][4]; }; 
            #endif
            //layout ( std430, binding = 5, set = 1 ) readonly buffer bvhMetaB { ivec4 bvhMeta[]; };
            layout ( binding = 5, set = 1 ) uniform isamplerBuffer bvhMeta;
            //#define TLOAD(img,t) texelFetch(img,t)
        #endif
        #endif
        
        layout ( std430, binding = 3, set = 1 ) readonly buffer geometryUniformB { GeometryUniformStruct geometryUniform;} geometryBlock;
        #ifdef VTX_TRANSPLIT // for leaf gens
            layout ( binding = 7, set = 1, rgba32f ) uniform imageBuffer lvtx;
            #define TLOAD(img,t) imageLoad(img,t)
            //layout ( std430, binding = 7, set = 1 )  buffer lvtxB { float lvtx[]; };
        #else
            layout ( binding = 8, set = 1 ) uniform samplerBuffer lvtx;
            #define TLOAD(img,t) texelFetch(img,t)
            //layout ( std430, binding = 7, set = 1 )  readonly buffer lvtxB { float lvtx[]; };
        #endif
    #endif
#endif

const int ATTRIB_EXTENT = 4;

// attribute formating
const int NORMAL_TID = 0;
const int TEXCOORD_TID = 1;
const int TANGENT_TID = 2;
const int BITANGENT_TID = 3;




//#define _SWIZV wzx
#define _SWIZV xyz

const int WARPED_WIDTH = 2048;
//const ivec2 mit[3] = {ivec2(0,0), ivec2(1,0), ivec2(0,1)};
const ivec2 mit[3] = {ivec2(0,1), ivec2(1,1), ivec2(1,0)};

ivec2 mosaicIdc(in ivec2 mosaicCoord, in int idc) {
#ifdef VERTEX_FILLING
    mosaicCoord.x %= int(imageSize(attrib_texture_out).x);
#endif
    return mosaicCoord + mit[idc];
}

ivec2 gatherMosaic(in ivec2 uniformCoord) {
    return ivec2(uniformCoord.x * 3 + uniformCoord.y % 3, uniformCoord.y);
}

vec4 fetchMosaic(in sampler2D vertices, in ivec2 mosaicCoord, in uint idc) {
    //return texelFetch(vertices, mosaicCoord + mit[idc], 0);
    return textureLod(vertices, (vec2(mosaicCoord + mit[idc]) + 0.49999f) / textureSize(vertices, 0), 0); // supper native warping
}

ivec2 getUniformCoord(in int indice) {
    return ivec2(indice % WARPED_WIDTH, indice / WARPED_WIDTH);
}


const mat3 uvwMap = mat3(vec3(1.f,0.f,0.f),vec3(0.f,1.f,0.f),vec3(0.f,0.f,1.f));

#ifndef VERTEX_FILLING
#ifndef BVH_CREATION
#ifdef ENABLE_VSTORAGE_DATA
float intersectTriangle(const vec3 orig, const mat3 M, const int axis, const int tri, inout vec2 UV, in bool valid) {
    float T = INFINITY;
    IFANY (valid) {
        // gather patterns
        const int itri = tri*3;//tri*9;
        const mat3 ABC = mat3(
            TLOAD(lvtx, itri+0).xyz-orig.xxx,
            TLOAD(lvtx, itri+1).xyz-orig.yyy,
            TLOAD(lvtx, itri+2).xyz-orig.zzz
            //vec3(lvtx[itri+0], lvtx[itri+1], lvtx[itri+2])-orig.x,
            //vec3(lvtx[itri+3], lvtx[itri+4], lvtx[itri+5])-orig.y,
            //vec3(lvtx[itri+6], lvtx[itri+7], lvtx[itri+8])-orig.z
        )*M;

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

float intersectTriangle(const vec3 orig, const vec3 dir, const int tri, inout vec2 uv, in bool _valid) {
    const int itri = tri*3;//tri*9;
    const mat3 vT = transpose(mat3(
        TLOAD(lvtx, itri+0).xyz,
        TLOAD(lvtx, itri+1).xyz,
        TLOAD(lvtx, itri+2).xyz
        //vec3(lvtx[itri+0], lvtx[itri+1], lvtx[itri+2]),
        //vec3(lvtx[itri+3], lvtx[itri+4], lvtx[itri+5]),
        //vec3(lvtx[itri+6], lvtx[itri+7], lvtx[itri+8])
    ));
    const vec3 e1 = vT[1]-vT[0], e2 = vT[2]-vT[0];
    const vec3 h = cross(dir, e2);
    const float a = dot(e1,h);

#ifdef BACKFACE_CULLING
    if (a < 1e-5f) { _valid = false; }
#else
    if (abs(a) < 1e-5f) { _valid = false; }
#endif

    const float f = 1.f/a;
    const vec3 s = orig - vT[0], q = cross(s, e1);
    uv = f * vec2(dot(s,h),dot(dir,q));

    if (uv.x < 0.f || uv.y < 0.f || (uv.x+uv.y) > 1.f) { _valid = false; }

    float T = f * dot(e2,q);
    if (T >= INFINITY || T < 0.f) { _valid = false; } 
    if (!_valid) T = INFINITY;
    return T;
}


#endif
#endif
#endif



const int _BVH_WIDTH = 2048;

/*
#define bvhT_ptr ivec2
bvhT_ptr mk_bvhT_ptr(in int linear) {
    //int md = linear & 1; linear >>= 1;
    //return bvhT_ptr(linear % _BVH_WIDTH, ((linear / _BVH_WIDTH) << 1) + md);
    return bvhT_ptr(linear % _BVH_WIDTH, linear / _BVH_WIDTH); // just make linear (gather by tops of...)
}*/


#ifdef ENABLE_VSTORAGE_DATA
#ifdef ENABLE_VERTEX_INTERPOLATOR
// barycentric map (for corrections tangents in POM)
void interpolateMeshData(inout HitData ht) {
    const int tri = floatBitsToInt(ht.uvt.w)-1; 
    const vec3 vs = vec3(1.0f - ht.uvt.x - ht.uvt.y, ht.uvt.xy); 
    const vec2 sz = 1.f.xx / textureSize(attrib_texture, 0), szt = sz * 0.9999f;
    const bool_ validInterpolant = greaterEqualF(ht.uvt.z, 0.0f) & lessF(ht.uvt.z, INFINITY) & bool_(tri >= 0) & bool_(materials[tri] == ht.materialID);

    IF (validInterpolant) {
        vec2 trig = (fma(vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+ TEXCOORD_TID))), sz, szt));
        vec2 txcd = vs * mat2x3(SGATHER(attrib_texture, trig, 0)._SWIZV, SGATHER(attrib_texture, trig, 1)._SWIZV);

        trig = (fma(vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+   NORMAL_TID))), sz, szt));
        vec3 nrml = normalize(vs * mat3x3(SGATHER(attrib_texture, trig, 0)._SWIZV, SGATHER(attrib_texture, trig, 1)._SWIZV, SGATHER(attrib_texture, trig, 2)._SWIZV));

        trig = (fma(vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+  TANGENT_TID))), sz, szt));
        vec3 tngt = normalize(vs * mat3x3(SGATHER(attrib_texture, trig, 0)._SWIZV, SGATHER(attrib_texture, trig, 1)._SWIZV, SGATHER(attrib_texture, trig, 2)._SWIZV));

        trig = (fma(vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+BITANGENT_TID))), sz, szt));
        vec3 btng = normalize(vs * mat3x3(SGATHER(attrib_texture, trig, 0)._SWIZV, SGATHER(attrib_texture, trig, 1)._SWIZV, SGATHER(attrib_texture, trig, 2)._SWIZV));

        ht.texcoord.xy = txcd;
        ht.normal.xyz  = nrml;
        ht.tangent.xyz = tngt;
        ht.bitangent.xyz = btng;
    }
}
#endif
#endif

#endif
