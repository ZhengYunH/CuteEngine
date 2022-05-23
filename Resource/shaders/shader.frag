#version 450
#include "shader.zsh"


layout(binding = 1) uniform sampler2D texSampler;
layout(set=0, binding = 2) uniform LightData {
    DirLight dirLight;
    int nr_of_point_lights;
    PointLight pointLights[NR_POINT_LIGHTS];
    SpotLight spotLight;
}lightData;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragViewPos;
layout(location = 4) in vec3 fragPos;


layout(location = 0) out vec4 outColor;

vec3 CalcDirectionalLight(DirLight light)
{
    vec3 ambient = light.ambient.rgb;
    float diff = max(dot(fragNormal, light.direction), 0.0);
    vec3 diffuse = diff * light.diffuse.rgb;

    vec3 viewDir = normalize(fragViewPos - fragPos);
    vec3 reflectDir = reflect(-light.direction, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = spec * light.specular.rgb;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light)
{
    vec3 ambient = light.ambient.rgb;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse.rgb;

    vec3 viewDir = normalize(fragViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = spec * light.specular.rgb;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light)
{
    vec3 ambient = light.ambient.rgb;

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    if(theta <= light.outCutoff) // outside spot light
    {
        return ambient;
    }
    float epsilon = light.cutoff - light.outCutoff;
    float intensity = clamp((theta - light.outCutoff) / epsilon, 0.0, 1.0);

    float diff = max(dot(fragNormal, lightDir), 0.0) * intensity;
    vec3 diffuse = diff * light.diffuse.rgb;

    vec3 viewDir = normalize(fragViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16) * intensity;
    vec3 specular = spec * light.specular.rgb;

    return ambient + diffuse + specular;
}

void main() {
    vec3 outputColor = vec3(0, 0, 0);
    outputColor += CalcDirectionalLight(lightData.dirLight);
    for(int i = 0; i < lightData.nr_of_point_lights; i++)
        outputColor += CalcPointLight(lightData.pointLights[i]);
    outputColor += CalcSpotLight(lightData.spotLight);
    outColor = vec4(outputColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}