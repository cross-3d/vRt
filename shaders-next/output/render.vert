#version 460 core
#extension GL_GOOGLE_include_directive : enable

const vec2 cpositions[4] = { vec2(-1.f), vec2(1.f, -1.f), vec2(-1.f, 1.f), vec2(1.f) };
const vec2 tcoords[4] = { vec2(0.f), vec2(1.f, 0.f), vec2(0.f, 1.f), vec2(1.f) };

const mat3 rmat = mat3(
    1.f,  0.f,  0.f, 
    0.f, -1.f,  0.f,
    0.f,  0.f, -1.f
);

layout ( location = 0 ) out vec2 vcoord;

void main() {
    gl_Position = vec4(rmat*vec3(cpositions[gl_VertexIndex].xy, 0.0f), 1.0f);
    vcoord = tcoords[gl_VertexIndex].xy;
}
