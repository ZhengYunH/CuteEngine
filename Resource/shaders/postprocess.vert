#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;

void main() 
{
    vec3 vPos = inPos.xyz;
    gl_Position = vec4(vPos.xy*2-1, vPos.z, 1.0);
    outUV = inUV;
}
