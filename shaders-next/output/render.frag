#version 460 core
#extension GL_GOOGLE_include_directive : enable
#define FRAGMENT_SHADER
#define SIMPLIFIED_RAY_MANAGMENT

#include "../include/driver.glsl"

precision highp float;
precision highp int;

#include "../include/structs.glsl"

layout ( location = 0 ) out vec4 outFragColor;
layout ( location = 0 ) in vec2 vcoord;
layout ( binding = 0 ) uniform sampler2D samples;

#define textureFixed(tx) textureLod(samples, clamp(tx.xy,0.f.xx,1.f.xx), 0)
//vec4 filtered(in vec2 tx) { return textureFixed(tx); }

vec4 filtered(in vec2 tx) {
    /*
    vec2 hs = 1.f/textureSize(samples, 0);
    vec4 rf = textureFixed(tx);
    vec4 hz = (textureFixed(tx+vec2(hs.x,0.f))+textureFixed(tx+vec2(0.f,hs.y))+textureFixed(tx+vec2(-hs.x,0.f))+textureFixed(tx+vec2(0.f,-hs.y)))*0.25f;
    vec4 dz = (textureFixed(tx+vec2(hs.x,hs.y))+textureFixed(tx+vec2(-hs.x,hs.y))+textureFixed(tx+vec2(-hs.x,-hs.y))+textureFixed(tx+vec2(hs.x,-hs.y)))*0.25f;

    //return hz*0.5f + rf*0.5f;
    return max(hz, rf);
    */
    return textureFixed(tx);
}

void main() {
    outFragColor = vec4(filtered(vcoord.xy).xyz, 1.0f);
    //outFragColor = vec4(fromLinear(filtered(vcoord.xy)).xyz, 1.0f);
    //outFragColor = vec4(0.2, 0.6f, 0.9f, 1.f); // working test
}
