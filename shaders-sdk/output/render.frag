#version 460 core
#extension GL_GOOGLE_include_directive : enable
#define FRAGMENT_SHADER
#define SIMPLIFIED_RAY_MANAGMENT

precision highp float;
precision highp int;

#include "../include/driver.glsl"
#include "../include/mathlib.glsl"

layout ( location = 0 ) out vec4 outFragColor;
layout ( location = 0 ) in vec2 vcoord;
layout ( binding = 0 ) uniform sampler2D samples;

#define icolor(tx) textureLod(samples, tx.xy, 0)

void main() {
    outFragColor = point4(fromLinear(icolor(vcoord.xy)*2.f));
}
