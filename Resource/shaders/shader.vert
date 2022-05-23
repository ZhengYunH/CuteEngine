#version 450
#include "shader.zsh"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 viewPos;
layout(location = 4) out vec3 fragPos;


void main() {
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    gl_Position =  mvp * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal; // translate to WS
    fragTexCoord = inTexCoord;

    viewPos = vec3(ubo.view[3]);
    fragPos = inPosition;
}