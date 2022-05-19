#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragLightColor;
layout(location = 4) in vec3 fragLightDirection;
layout(location = 5) in vec3 fragViewPos;
layout(location = 6) in vec3 fragPos;



layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
    float diff = max(dot(fragNormal, fragLightDirection), 0.0);
    vec3 diffuse = diff * fragLightColor.rgb;
    vec3 ambient = fragLightColor.a * fragLightColor.rgb;

    // specular
    float specularStrength = 0.15;
    vec3 viewDir = normalize(fragViewPos - fragPos);
    vec3 reflectDir = reflect(-fragLightDirection, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * fragLightColor.rgb;

    outColor = vec4((ambient +  diffuse  + specular) * outColor.rgb, outColor.a);
}