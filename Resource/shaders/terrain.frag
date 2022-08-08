#version 450
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragNormal;

void main() {
    vec3 outputColor = normalize(fragNormal);
    outColor = vec4(outputColor, 1.0);
}
