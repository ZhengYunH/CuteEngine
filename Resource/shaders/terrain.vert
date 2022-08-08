#version 450
#include "shader.zsh"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} Batch;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;			
layout(location = 2) in vec2 inTexCoord;


layout(location = 0) out vec3 fragNormal;

void main() {
    mat4 mvp = Batch.proj * Batch.view * Batch.model;
    gl_Position =  mvp * vec4(inPosition, 1.0);
	fragNormal = inNormal;
}
