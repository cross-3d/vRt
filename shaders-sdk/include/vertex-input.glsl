#ifndef _VERTEX_INPUT_H
#define _VERTEX_INPUT_H


#ifdef VERTEX_FILLING
layout ( binding = 3, set = VTX_SET, align_ssbo ) coherent buffer VTX_BUFFER_IN { f32vec4 data[]; } lvtxIn[];
#endif


// buffer region
struct VtBufferRegion {
    uint32_t byteOffset, byteSize;
};

// subdata structuring in buffer region
struct VtBufferView {
    uint32_t regionID, byteStride;
    uint32_t byteOffset, byteLength;
};

// accessor
struct VtAccessor {
    uint32_t bufferViewID; // buffer-view structure
    uint32_t byteOffset; // accessor byteOffset
    uint32_t bitfield; // VtFormat decodable
};

// attribute binding
struct VtAttributeBinding {
    int32_t attributeID;
    int32_t accessorID;
};



const lowp ivec2 COMPONENTS = {0, 2}, ATYPE = {2, 4}, NORMALIZED = {6, 1};

int aComponents(in uint bitfield) { return int(vtParameteri(COMPONENTS, bitfield)); };
int aNormalized(in uint bitfield) { return int(vtParameteri(NORMALIZED, bitfield)); };
int aType(in uint bitfield) { return int(vtParameteri(ATYPE, bitfield)); };


#define BFS uint(bufferID)

//#ifdef ENABLE_INT16_SUPPORT
//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { u16vec2 data[]; } bufferSpace[];
//#else
//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint data[]; } bufferSpace[];
//#endif

//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint8_t data[]; } bufferSpace[];
//#ifdef ENABLE_INT16_SUPPORT
  layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint16_t data[]; } bufferSpace[];
//#else
//layout ( binding = 0, set = 1, align_ssbo ) readonly buffer VT_VINPUT { uint     data[]; } bufferSpace[]; // legacy GPU's
//#endif

layout ( binding = 2, set = 1, align_ssbo ) readonly buffer VT_BUFFER_VIEW { VtBufferView bufferViews[]; };
layout ( binding = 3, set = 1, align_ssbo ) readonly buffer VT_ACCESSOR { VtAccessor accessors[]; };
layout ( binding = 4, set = 1, align_ssbo ) readonly buffer VT_ATTRIB { VtAttributeBinding attributes[]; };

// First in world ByteAddressBuffer in Vulkan API by Ispanec (tested in RTX 2070 only)

//u16x1_t M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI; return u8x2pack(u8x2_t(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u])); };
//u32x1_t M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI; return u8x4pack(u8x4_t(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u],bufferSpace[BSC].data[Ot+2u],bufferSpace[BSC].data[Ot+3u])); };

u16x1_t M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return u16x1_t(bufferSpace[BSC].data[Ot+0u]); };
u32x1_t M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return u16x2pack(u16x2_t(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u])); };

//#ifdef ENABLE_INT16_SUPPORT // 16-bit wide version, optimized (with RX Vega support) 
//uint16_t M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return uint16_t(bufferSpace[BSC].data[Ot+0u]); };
//uint32_t M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return pack32(u16vec2(bufferSpace[BSC].data[Ot+0u],bufferSpace[BSC].data[Ot+1u])); };
//#else // for legacy GPU support
//const lowp ivec2 b16m = {0,16};
//highp uint M16(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=1; return bitfieldExtract(bufferSpace[BSC].data[Ot>>1u],b16m[Ot&1u],16); };
//      uint M32(in NonUniform uint BSC, in uint Ot, in uint uI) { Ot+=uI,Ot>>=2; return bufferSpace[BSC].data[Ot]; };
//#endif


struct VtVIUniform {
    uint32_t primitiveOffset, primitiveCount, attributeOffset, attributeCount;
     int32_t verticeAccessor, indiceAccessor, materialAccessor;
    uint32_t bitfield, materialID, readOffset;
};


// uniform input of vertex loader
layout ( binding = 9, set = VTX_SET, align_ssbo ) readonly buffer VT_UNIFORM { VtVIUniform _vertexBlock[]; };
layout ( push_constant ) uniform VT_CONSTS { uint inputID; } cblock;
#define vertexBlock _vertexBlock[gl_GlobalInvocationID.y + cblock.inputID]
layout ( binding = 6, set = 1, align_ssbo ) readonly buffer VT_TRANSFORMS { f32mat3x4 vTransforms[]; };


uint calculateByteOffset(in int accessorID, in const uint bytecorrect) { //bytecorrect -= 1;
    const uint bufferView = uint(accessors[accessorID].bufferViewID);
    return ((bufferViews[bufferView].byteOffset+accessors[accessorID].byteOffset) >> bytecorrect);
};

uint iCR(in int accessorID, in uint index, in const uint cmpc, in const uint bytecorrect) {
    const uint bufferView = uint(accessors[accessorID].bufferViewID), stride = max(bufferViews[bufferView].byteStride, (aComponents(accessors[accessorID].bitfield)+1)<<bytecorrect);
    return (index*stride)+(cmpc<<bytecorrect);
};

void readByAccessorLL(in int accessor, in uint index, inout u32vec4 outpx) {
    [[flatten]] if (accessor >= 0) {
        const uint bufferID = bufferViews[accessors[accessor].bufferViewID].regionID;
        const uint T = calculateByteOffset(accessor, 0u), D = 0u, C = min(aComponents(accessors[accessor].bitfield)+1, 4u-D);
        [[unroll]] for (int i=0;i<4;i++) { [[flatten]] if (C > i) outpx[D+i] = M32(BFS,T,iCR(accessor,index,i,2u)); };
    };
};

// 
u32vec4 readByAccessorLLW(in int accessor, in uint index, in u32vec4 outpx) { readByAccessorLL(accessor, index, outpx); return outpx; };

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
        const uint bufferID = bufferViews[accessors[accessor].bufferViewID].regionID;
        const bool U16 = aType(accessors[accessor].bitfield) == 2; // uint16
        const uint T = calculateByteOffset(accessor, 0u);
        [[flatten]] if (U16) { outp = M16(BFS,T,iCR(accessor,index,0,1u)); } else { outp = M32(BFS,T,iCR(accessor,index,0,2u)); };
    };
};

// 
void storePosition(in ivec2 cdata, in vec4 fval) {
    const uint inputID = gl_GlobalInvocationID.y + uint(cblock.inputID);
    fval.xyz = mult4(vTransforms[inputID], fval);
    lvtxIn[0].data[cdata.x*3+cdata.y] = fval;
};


#endif
