#version 460 core
#extension GL_GOOGLE_include_directive : enable
#define FRAGMENT_SHADER
#define SIMPLIFIED_RAY_MANAGMENT

#include "../include/driver.glsl"

precision highp float;
precision highp int;

layout ( location = 0 ) out vec4 outFragColor;
layout ( location = 0 ) in vec2 vcoord;
layout ( binding = 0 ) uniform sampler2D samples;

#define icolor(tx) textureLod(samples, tx.xy, 0)

void main() {
    outFragColor = vec4(icolor(vcoord.xy).xyz, 1.0f);
}
