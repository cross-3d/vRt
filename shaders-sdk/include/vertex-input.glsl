#ifndef _VERTEX_INPUT_H
#define _VERTEX_INPUT_H







#ifdef ENABLE_VEGA_INSTRUCTION_SET

#ifdef ENABLE_INT16_SUPPORT // native 16-bit integer support
uint16_t M16(in f16samplerBuffer m, in uint i) {
    return float16BitsToUint16(texelFetch(m, int(i>>1)).xy)[i&1u];
};
#else
highp uint M16(in f16samplerBuffer m, in uint i) {
    return bitfieldExtract(packFloat2x16(texelFetch(m, int(i>>1)).xy), int(i&1)<<4, 16); // unified sentence of uint16_t values
};
#endif

uint M32(in f16samplerBuffer m, in uint i) { 
    return packFloat2x16(texelFetch(m, int(i)).xy);
};
#endif

highp uint M16(in highp usamplerBuffer m, in uint i) {
    return texelFetch(m, int(i>>1))[i&1];
};

uint M32(in highp usamplerBuffer m, in uint i) {
    const highp uvec2 mpc = texelFetch(m, int(i)).xy;
    return ((mpc.y<<16u)|mpc.x);
};

/*
highp uint M16(in mediump samplerBuffer m, in uint i) {
    const highp uvec2 mpc = floatBitsToUint(texelFetch(m, int(i>>1)).xy);
    return floatBitsToUint(texelFetch(m, int(i>>1)).xy)[i&1];
}

uint M32(in mediump samplerBuffer m, in uint i) {
    //return packHalf2x16(texelFetch(m, int(i)).xy); // inaccurate 
    const highp uvec2 mpc = floatBitsToUint(texelFetch(m, int(i)).xy);
    return ((mpc.y<<16u)|mpc.x);
}
*/

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




#define BFS bufferSpace[nonuniformEXT(bufferID)]



#ifdef ENABLE_VEGA_INSTRUCTION_SET
layout ( binding = 0, set = 1 ) uniform f16samplerBuffer bufferSpace[8]; // vertex model v1.4
#else
layout ( binding = 0, set = 1 ) uniform highp usamplerBuffer bufferSpace[8]; // vertex model v1.4
#endif

//layout ( binding = 0, set = 1 ) uniform mediump samplerBuffer bufferSpace[8];

layout ( binding = 2, set = 1, std430 ) readonly buffer VT_BUFFER_VIEW {VtBufferView bufferViews[]; };
layout ( binding = 3, set = 1, std430 ) readonly buffer VT_ACCESSOR {VtAccessor accessors[]; };
layout ( binding = 4, set = 1, std430 ) readonly buffer VT_ATTRIB {VtAttributeBinding attributes[]; };



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
layout ( binding = 5, set = 1, std430 ) readonly buffer VT_UNIFORM { VtVIUniform _vertexBlock[]; };
layout ( push_constant ) uniform VT_CONSTS { uint inputID; } cblock;
#define vertexBlock _vertexBlock[gl_GlobalInvocationID.y + cblock.inputID]
layout ( binding = 6, set = 1, std430 ) readonly buffer VT_TRANSFORMS { mat3x4 vTransforms[]; };


uint calculateByteOffset(in int accessorID, in uint index, in uint bytecorrect) { //bytecorrect -= 1;
    const uint bufferView = uint(accessors[accessorID].bufferView);
    const uint stride = max(bufferViews[bufferView].byteStride, (aComponents(accessors[accessorID].bitfield)+1) << bytecorrect); // get true stride 
    const uint offseT = index * stride + (bufferViews[bufferView].byteOffset+accessors[accessorID].byteOffset); // calculate byte offset 
    return offseT >> bytecorrect;
};

void readByAccessorLL(in int accessor, in uint index, inout uvec4 outpx) {
    uint attribution[4] = {outpx.x, outpx.y, outpx.z, outpx.w};
    [[flatten]] if (accessor >= 0) {
        const int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const uint T = calculateByteOffset(accessor, index, 2);
        const uint C = aComponents(accessors[accessor].bitfield)+1;
        const uint D = 0u; // component decoration
        [[unroll]] for (int i=0;i<4;i++) {
            [[flatten]] if (C > i) attribution[D+i] = M32(BFS,T+i);
        };
    }
    outpx = uvec4(attribution[0], attribution[1], attribution[2], attribution[3]);
};

uvec4 readByAccessorLLW(in int accessor, in uint index, in uvec4 outpx) { readByAccessorLL(accessor, index, outpx); return outpx; };


// vec4 getter
void readByAccessor(in int accessor, in uint index, inout vec4 outp) {
    outp.xyzw = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(outp)).xyzw);
}

// vec3 getter
void readByAccessor(in int accessor, in uint index, inout vec3 outp) {
    outp.xyz = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.x))).xyz);
}

// vec2 getter
void readByAccessor(in int accessor, in uint index, inout vec2 outp) {
    outp.xy = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.xx))).xy);
}

// vec1 getter
void readByAccessor(in int accessor, in uint index, inout float outp) {
    outp.x = uintBitsToFloat(readByAccessorLLW(accessor, index, floatBitsToUint(vec4(outp, 0.f.xxx))).x);
}

// ivec1 getter
void readByAccessor(in int accessor, in uint index, inout int outp) {
    outp.x = int(readByAccessorLLW(accessor, index, uvec4(outp, 0u.xxx)).x);
}

// uvec1 getter
void readByAccessor(in int accessor, in uint index, inout uint outp) {
    outp.x = readByAccessorLLW(accessor, index, uvec4(outp, 0u.xxx)).x;
}



// planned read type directly from accessor
void readByAccessorIndice(in int accessor, in uint index, inout uint outp) {
    [[flatten]] if (accessor >= 0) {
        const int bufferID = bufferViews[accessors[accessor].bufferView].regionID;
        const bool U16 = aType(accessors[accessor].bitfield) == 2; // uint16
        const uint T = calculateByteOffset(accessor, index, U16 ? 1 : 2);
        [[flatten]] if (U16) { outp = M16(BFS,T+0); } else { outp = M32(BFS,T+0); }
    }
}

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
    const uint inputID = gl_GlobalInvocationID.y + uint(cblock.inputID);
    fval.xyz = mult4(vTransforms[inputID], fval);
    ISTORE(lvtxIn, cdata.x*3+cdata.y, fval);
}


#endif
