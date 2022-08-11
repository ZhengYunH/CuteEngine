#version 450
#include "shader.zsh"


layout(binding = 1) uniform sampler2D texSampler;
layout(set=0, binding = 2) uniform LightData {
    DirLight dirLight;
    int nr_of_point_lights;
    PointLight pointLights[NR_POINT_LIGHTS];
    SpotLight spotLight;
}Light;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragViewPos;
layout(location = 4) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0,0,0,0);
}
