#ifndef _VERTEX_INPUT_H
#define _VERTEX_INPUT_H


#ifdef VERTEX_FILLING
layout ( binding = 3, set = VTX_SET, rgba32f ) coherent uniform imageBuffer lvtxIn;
layout ( binding = 8, set = VTX_SET, r32ui ) coherent uniform uimageBuffer indexI;
#endif


#if defined(ENABLE_VEGA_INSTRUCTION_SET) && defined(ENABLE_FP16_SAMPLER_HACK) && defined(ENABLE_FP16_SUPPORT)
    #ifdef ENABLE_INT16_SUPPORT // native 16-bit integer support
    uint16_t M16(in f16samplerBuffer m, in uint i) {
        return float16BitsToUint16(texelFetch(m, int(i>>1u)).xy)[i&1u];
    };
    #else
    highp uint M16(in f16samplerBuffer m, in uint i) {
        return bitfieldExtract(packFloat2x16(texelFetch(m, int(i>>1u)).xy), int(i&1u)<<4, 16); // unified sentence of uint16_t values
    };
    #endif
    uint M32(in f16samplerBuffer m, in uint i) { 
        return packFloat2x16(texelFetch(m, int(i)).xy);
    };
#endif


//highp uint M16(in highp usamplerBuffer m, in uint i) { return texelFetch(m, int(i>>1u))[i&1u]; };
//uint M32(in highp usamplerBuffer m, in uint i) { return p2x_16(texelFetch(m, int(i)).xy); };

//highp uint M16(in highp usamplerBuffer m, in uint i) { return texelFetch(m, int(i>>1u))[i&1u]; };
//uint M32(in highp usamplerBuffer m, in uint i) { return p2x_16(texelFetch(m, int(i)).xy); };



// buffer region
struct VtBufferRegion {
    uint byteOffset;
    uint byteSize;
};

// subdata structuring in buffer region
struct VtBufferView {
    int regionID;
    int byteOffset; // in structure offset
    int byteStride;
    int byteLength;
};

// accessor
struct VtAccessor {
    int bufferView; // buffer-view structure
    int byteOffset; // accessor byteOffset
    uint bitfield; // VtFormat decodable
    uint reserved;
};

// attribute binding
struct VtAttributeBinding {
    int attributeID;
    int accessorID;
    //int indexOffset;
};



const ivec2 COMPONENTS = ivec2(0, 2);
const ivec2 ATYPE = ivec2(2, 4);
const ivec2 NORMALIZED = ivec2(6, 1);

int aComponents(in uint bitfield) { return int(parameteri(COMPONENTS, bitfield)); };
int aNormalized(in uint bitfield) { return int(parameteri(NORMALIZED, bitfield)); };
int aType(in uint bitfield) { return int(parameteri(ATYPE, bitfield)); };


//#ifdef ENABLE_NON_UNIFORM_SAMPLER
//#define BFS bufferSpace[NonUniform(bufferID)]
//#else
//#define BFS bufferSpace[bufferID]
//#endif
#define BFS uint(bufferID)


//#if defined(ENABLE_VEGA_INSTRUCTION_SET) && defined(ENABLE_FP16_SUPPORT) && defined(ENABLE_FP16_SAMPLER_HACK)
//layout ( binding = 0, set = 1 )  uniform f16samplerBuffer bufferSpace[8]; // 
//#else
//layout ( binding = 0, set = 1 )  uniform highp usamplerBuffer bufferSpace[8]; // 
//#endif

layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { u16vec2 data[]; } bufferSpace[];
layout ( binding = 2, set = 1, align_ssbo ) readonly buffer VT_BUFFER_VIEW { VtBufferView bufferViews[]; };
layout ( binding = 3, set = 1, align_ssbo ) readonly buffer VT_ACCESSOR { VtAccessor accessors[]; };
layout ( binding = 4, set = 1, align_ssbo ) readonly buffer VT_ATTRIB { VtAttributeBinding attributes[]; };


//
highp uint M16(in uint BSC, in uint Ot, in uint uI) {
    const uint I = (Ot+uI)>>1u;
    return bufferSpace[NonUniform(BSC)].data[I>>1u][I&1u]; 
};

uint M32(in uint BSC, in uint Ot, in uint uI) {
    const uint I = (Ot+uI)>>2u;
    return p2x_16(bufferSpace[NonUniform(BSC)].data[I]); 
};



struct VtVIUniform {
    uint primitiveCount;
    int verticeAccessor;
    int indiceAccessor;
    int materialAccessor;

    uint primitiveOffset;
    uint attributeOffset;
    uint attributeCount;
    uint bitfield;

    uint materialID;
    uint readOffset;
    uint reserved0, reserved1;
};

// uniform input of vertex loader
//layout ( binding = 5, set = 1, align_ssbo ) readonly buffer VT_UNIFORM { VtVIUniform _vertexBlock[]; };
layout ( binding = 9, set = VTX_SET, align_ssbo ) readonly buffer VT_UNIFORM { VtVIUniform _vertexBlock[]; };
layout ( push_constant ) uniform VT_CONSTS { uint inputID; } cblock;
#define vertexBlock _vertexBlock[gl_GlobalInvocationID.y + cblock.inputID]
layout ( binding = 6, set = 1, align_ssbo ) readonly buffer VT_TRANSFORMS { mat3x4 vTransforms[]; };


uint calculateByteOffset(in int accessorID, in const uint bytecorrect) { //bytecorrect -= 1;
    const uint bufferView = uint(accessors[accessorID].bufferView);
    return ((bufferViews[bufferView].byteOffset+accessors[accessorID].byteOffset) >> bytecorrect);
};

uint iCR(in int accessorID, in uint index, in const uint cmpc, in const uint bytecorrect) {
    const uint bufferView = uint(accessors[accessorID].bufferView), stride = max(bufferViews[bufferView].byteStride, (aComponents(accessors[accessorID].bitfield)+1)<<bytecorrect);
    return (index*stride)+(cmpc<<bytecorrect);
};

void readByAccessorLL(in int accessor, in uint index, inout uvec4 outpx) {
    [[flatten]] if (accessor >= 0) {
        const int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const uint T = calculateByteOffset(accessor, 0u), D = 0u, C = min(aComponents(accessors[accessor].bitfield)+1, 4u-D);
        [[unroll]] for (int i=0;i<4;i++) { [[flatten]] if (C > i) outpx[D+i] = M32(BFS,T,iCR(accessor,index,i,2u)); };
    };
};

// 
uvec4 readByAccessorLLW(in int accessor, in uint index, in uvec4 outpx) { readByAccessorLL(accessor, index, outpx); return outpx; };

// vec4 getter
void readByAccessor(in int accessor, in uint index, inout vec4 outp) {
    outp.xyzw = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(outp)).xyzw);
};

// vec3 getter
void readByAccessor(in int accessor, in uint index, inout vec3 outp) {
    outp.xyz = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.x))).xyz);
};

// vec2 getter
void readByAccessor(in int accessor, in uint index, inout vec2 outp) {
    outp.xy = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.xx))).xy);
};

// vec1 getter
void readByAccessor(in int accessor, in uint index, inout float outp) {
    outp.x = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.xxx))).x);
};

// ivec1 getter
void readByAccessor(in int accessor, in uint index, inout int outp) {
    outp.x = int(readByAccessorLLW(accessor, index, uvec4(outp, 0u.xxx)).x);
};

// uvec1 getter
void readByAccessor(in int accessor, in uint index, inout uint outp) {
    outp.x = readByAccessorLLW(accessor, index, uvec4(outp, 0u.xxx)).x;
};


// planned read type directly from accessor
void readByAccessorIndice(in int accessor, in uint index, inout uint outp) {
    [[flatten]] if (accessor >= 0) {
        const int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const bool U16 = aType(accessors[accessor].bitfield) == 2; // uint16
        const uint T = calculateByteOffset(accessor, 0u);
        [[flatten]] if (U16) { outp = M16(BFS,T,iCR(accessor,index,0,1u)); } else { outp = M32(BFS,T,iCR(accessor,index,0,2u)); };
    };
};

// 
void storePosition(in ivec2 cdata, in vec4 fval) {
    const uint inputID = gl_GlobalInvocationID.y + uint(cblock.inputID);
    fval.xyz = mult4(vTransforms[inputID], fval);
    ISTORE(indexI, cdata.x*3+cdata.y, uint(cdata.x*3+cdata.y).xxxx);
    ISTORE(lvtxIn, cdata.x*3+cdata.y, fval);
};


#endif
