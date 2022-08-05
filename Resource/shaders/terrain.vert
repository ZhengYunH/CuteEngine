#version 450
#include "shader.zsh"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;


void main() {
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    gl_Position =  mvp * vec4(inPosition, 1.0);
}