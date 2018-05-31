const float maxHdrExposure = 1.f;

layout ( binding = 20, set = 0 ) uniform sampler2D skybox[1]; // united state with ray tracing

vec4 readEnv(in vec2 ds) {
    vec2 tx2 = ((ds / PI - vec2(0.5f,0.0f)) * vec2(0.5f,1.0f)); tx2.y=1.f-tx2.y;
    return texture(skybox[0], tx2);
}

void env(inout vec4 color, in RayRework ray) {
    color = readEnv(ray.cdirect.xy);
    color = clamp(color, vec4(0.f.xxxx), vec4(maxHdrExposure.xxx,1.f));
}

#define EnvironmentShader env
