#version 460 core

#extension GL_GOOGLE_include_directive : enable

#define FRAGMENT_SHADER
#define SIMPLIFIED_RAY_MANAGMENT

#include "../include/driver.glsl"

precision highp float;
precision highp int;

#include "../include/structs.glsl"

layout ( location = 0 ) out vec4 outFragColor;
layout ( binding = 0 ) uniform sampler2D samples;
layout ( location = 0 ) in vec2 vcoord;

#define textureFixed(tx) textureLod(samples,(clamp(tx.xy,0.f.xx,1.f.xx)*vec2(1.f,0.5f)+vec2(0.f,0.5f)),0)

vec4 filtered(in vec2 tx) {
    return textureFixed(tx);
}

void main() {
    vec2 ctx = vcoord.xy;
    vec2 tsz = textureSize(samples, 0)*vec2(1.f,0.5f), cts = ctx * tsz;
    outFragColor = vec4(fromLinear(filtered(ctx)).xyz, 1.0f);
}
