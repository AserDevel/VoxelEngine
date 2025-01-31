#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout (location = 2) in uint materialID;
layout (location = 3) in uint AOvalue;

uniform mat4 matCamera;
uniform mat4 localSpaceMatrix; // Same light space matrix used for shadow map
uniform mat4 globalSpaceMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLocal;
out vec4 FragPosGlobal;
out uint MatID;
out float AO;

void main() {
    Normal = normalize(normal);
    FragPos = position;
    FragPosLocal = localSpaceMatrix * vec4(position, 1.0);
    FragPosGlobal = globalSpaceMatrix * vec4(position, 1.0);

    gl_Position = matCamera * vec4(position, 1.0);

    MatID = materialID;
    AO = float(AOvalue);
}

#shader fragment
#version 460 core

struct Material {
    vec3 color;
    float reflectivity;
    uint shininess;
};

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLocal;
in vec4 FragPosGlobal;
flat in uint MatID;
in float AO;

uniform vec3 eyePos;
uniform float globalAmbience;
uniform vec3 directionalLightDir;
uniform vec3 directionalLightColor;
uniform Material materials[256];
uniform sampler2D localShadowMap;
uniform sampler2D globalShadowMap;

out vec4 fragColor;

float ShadowCalculation(sampler2D shadowMap, vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range

    if (projCoords.z > 1.0f) // Outside shadow map depth
        return 0.0f;

    float closestDepth = texture(shadowMap, projCoords.xy).r; // Depth from shadow map
    float currentDepth = projCoords.z; // Fragment's depth in light space

    // Shadow comparison with bias
    float shadow = currentDepth > closestDepth ? 0.5 : 0.0;

    return shadow;
}


float ShadowCalculationSoft(sampler2D shadowMap, vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range

    // Discard fragments outside the shadow map range
    if (projCoords.z > 1.0f)
        return 0.0f;

    // Define PCF kernel size
    float shadow = 0.0;
    int samples = 4; // Number of samples (2x2 kernel, adjustable)
    float offset = 1.0 / 4096.0; // Adjust based on shadow map resolution
    float bias = max(0.0005 * (1.0 - dot(Normal, normalize(-directionalLightDir))), 0.0005);

    // Loop through kernel
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            // Sample neighboring depth values
            vec2 offsetCoord = projCoords.xy + vec2(x, y) * offset;
            float closestDepth = texture(shadowMap, offsetCoord).r;
            // Compare depths
            shadow += projCoords.z >= closestDepth ? 1.0 : 0.0;
        }
    }

    // Average the results
    shadow /= float((samples + 1) * (samples + 1)); // Normalize by the number of samples

    return shadow;
}


/*
float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range

    if (projCoords.z > 1.0f) // Outside shadow map depth
        return 0.0f;

    float closestDepth = texture(shadowMap, projCoords.xy).r; // Depth from shadow map
    float currentDepth = projCoords.z; // Fragment's depth in light space

    // Calculate the distance between the current fragment and the light
    float shadowDistance = abs(currentDepth - closestDepth);

    // You can modify this to control the fade based on distance
    float fadeFactor = clamp(1.0 - shadowDistance * 10.0, 0.0, 0.5); // Adjust 10.0 to control fade rate

    // Shadow comparison with bias and apply the fade
    float shadow = (currentDepth > closestDepth + 0.00001f) ? fadeFactor : 0.0f;

    return shadow;
}
*/

void main() {
    // Directional variables
    vec3 eyeDir = normalize(eyePos - FragPos);

    // Add directional light (sun/moon)
    vec3 lightDir = normalize(-directionalLightDir); // Sunlight direction
    // ambient (the directional light has twice the ambience)
    vec3 ambient = globalAmbience * directionalLightColor * AO;
    // diffuse
    float exposureFactor = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = exposureFactor * directionalLightColor;
    // Specular
    vec3 halfwayDir = normalize(lightDir + eyeDir);
    vec3 specular = (materials[MatID].reflectivity * pow(max(dot(Normal, halfwayDir), 0.0), materials[MatID].shininess)) * directionalLightColor;

    vec3 result = ambient + diffuse + specular;  
    
    // Calculate shadow
    float localShadow = ShadowCalculationSoft(localShadowMap, FragPosLocal);
    float globalShadow = ShadowCalculation(globalShadowMap, FragPosGlobal);

    // Blend local and global shadow
    float distanceToCamera = length(eyePos - FragPos);
    float blendFactor = smoothstep(10.0, 15.0, distanceToCamera); // Adjust blend range

    float finalShadow = mix(localShadow, globalShadow, blendFactor);

    // Final color = ambient + diffuse + shadow
    vec3 color = materials[MatID].color * result * (1.0 - 0);
    fragColor = vec4(color, 1.0);
}
