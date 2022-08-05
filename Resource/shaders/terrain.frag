#version 450
layout(location = 0) out vec4 outColor;

void main() {
    vec3 outputColor = vec3(1, 0, 0);
    outColor = vec4(outputColor, 1.0);
}