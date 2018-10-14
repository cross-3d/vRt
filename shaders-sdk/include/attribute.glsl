#ifndef _ATTRIBUTE_H
#define _ATTRIBUTE_H

#include "../include/mathlib.glsl"

#ifdef ENABLE_VERTEX_INTERPOLATOR
#ifndef ENABLE_VSTORAGE_DATA
#define ENABLE_VSTORAGE_DATA
#endif
#endif


// attribute formating
const int NORMAL_TID = 0;
const int TEXCOORD_TID = 1;
const int TANGENT_TID = 2;
const int BITANGENT_TID = 3;
const int VCOLOR_TID = 4;
const int VCOUNT = 3;


// Geometry Zone
#if (defined(ENABLE_VSTORAGE_DATA) || defined(BVH_CREATION) || defined(VERTEX_FILLING))
    #ifndef VTX_SET
        #ifdef VERTEX_FILLING
        #define VTX_SET 0
        #else
        #define VTX_SET 2
        #endif
    #endif

    #ifdef VERTEX_FILLING
    layout ( binding = 1, set = VTX_SET, std430   ) buffer materialsB { int vmaterials[]; };
    layout ( binding = 4, set = VTX_SET, rgba32f  ) uniform highp image2D attrib_texture_out;
    #else
    layout ( binding = 1, set = VTX_SET, std430   ) readonly buffer materialsB { int vmaterials[]; };
    layout ( binding = 6, set = VTX_SET           ) uniform highp sampler2D attrib_texture;
    #endif
#endif


const int WARPED_WIDTH = 4096;
const ivec2 mit[4] = {ivec2(0,1), ivec2(1,1), ivec2(1,0), ivec2(0,0)};
ivec2 gatherMosaic(in ivec2 uniformCoord) {
    return ivec2(uniformCoord.x * 3 + (uniformCoord.y % 3), uniformCoord.y);
}

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


#ifdef VERTEX_FILLING
void storeAttribute(in ivec3 cdata, in vec4 fval) {
    const ivec2 ATTRIB_ = gatherMosaic(getUniformCoord(cdata.x*ATTRIB_EXTENT+cdata.y));
    [[flatten]] if (cdata.z < 3) {
        ISTORE(attrib_texture_out, mosaicIdc(ATTRIB_,cdata.z), (fval));
    }
}
#endif 


#if (defined(ENABLE_VSTORAGE_DATA) && defined(ENABLE_VERTEX_INTERPOLATOR))
// barycentric map (for corrections tangents in POM)
void interpolateMeshData(in int hitID, in int tri) {
    const vec4 vs = vec4(1.0f - hit.uvt.x - hit.uvt.y, hit.uvt.xy, 0.f); // added additional component for shared computing capable production 
    const vec2 sz = 1.f.xx / textureSize(attrib_texture, 0);
    [[flatten]] if (hit.attribID > 0) {
        //[[unroll]] for (int i=0;i<ATTRIB_EXTENT;i++) {
        [[dont_unroll]] for (int i=0;i<ATTRIB_EXTENT;i++) {
            const vec2 trig = (vec2(gatherMosaic(getUniformCoord(tri*ATTRIB_EXTENT+i))) + 0.5f) * sz;

            vec4 attrib = 0.f.xxxx;
            //[[unroll]] for (int j=0;j<3;j++) {attrib=fma(vs[j].xxxx,textureLod(attrib_texture,fma(sz,offsetf[j],trig),0),attrib);}; // using accumulation sequence
            [[unroll]] for (int j=0;j<4;j++) {attrib[j]=dot(vs,sifonGather(attrib_texture,trig,j));}; // tensor capable production 

            // TODO: support of variable attribute transforms
#ifdef EXPERIMENTAL_INSTANCING_SUPPORT
            [[flatten]] if (any(equal(i.xxx, ivec3(NORMAL_TID, TANGENT_TID, BITANGENT_TID)))) attrib = mult4(bvhInstance.transformIn, attrib);
#endif

            ISTORE(attributes, makeAttribID(hit.attribID, i), attrib);
        }
    }
}
#endif


#endif
