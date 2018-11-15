#version 460 core
#extension GL_NV_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable

struct VtCustomPayload {
     vec4 lastIntersection;
    ivec4 binaryData128;
};

                     hitAttributeNV vec2 attribs;
layout(location = 0) rayPayloadInNV VtCustomPayload primitiveState;

void main()
{
    primitiveState.lastIntersection = vec4(attribs.xy, gl_HitTNV, intBitsToFloat(gl_PrimitiveID+1));
    //primitiveState.lastIntersection = vec4(attribs.xy, gl_HitTNV, intBitsToFloat(1));
    primitiveState.binaryData128 = ivec4(gl_InstanceID, (-1).xxx);
};
