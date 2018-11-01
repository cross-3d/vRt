#version 460 core
#extension GL_NVX_raytracing : require
#extension GL_GOOGLE_include_directive : enable

struct VtCustomPayloadNVX {
    vec4 lastIntersection;
    uvec4 binaryData128;
};

layout(location = 0) hitAttributeNVX vec2 attribs;
layout(location = 0) rayPayloadInNVX VtCustomPayloadNVX primitiveState;

void main()
{
    primitiveState.lastIntersection = vec4(attribs.xy, gl_HitTNVX, intBitsToFloat(gl_PrimitiveID+1));
    //primitiveState.lastIntersection = vec4(attribs.xy, gl_HitTNVX, intBitsToFloat(1));
    primitiveState.binaryData128.x = gl_InstanceID;
};
