#shader vertex
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in uint texIndex;
layout (location = 4) in uint shininess;
layout (location = 5) in float reflectivity;
layout (location = 6) in mat4 matWorld;

out vec3 Normal;
out vec3 FragPos;

out vec2 textureCoord;
flat out uint textureIndex;

out uint Shininess;
out float Reflectivity;

uniform mat4 matCamera;

void main() {
    // Calculate the models projected position
    gl_Position = matCamera * matWorld * vec4(position, 1.0);

    // Transform normal and frag position to world space
    FragPos = vec3(matWorld * vec4(position, 1.0));
    Normal = vec3(transpose(inverse(matWorld)) * vec4(normal, 0.0));

    // Transfer texture data
    textureCoord = texCoord;
    textureIndex = texIndex;

    Shininess = shininess;
    Reflectivity = reflectivity;
}


#shader fragment
#version 460 core

struct Light {
    vec3 position;
    vec3 color;
    float intensity;

    // Auttenuation factors
    float constant;
    float linear;
    float quadratic;
};

uniform int numLights;
uniform Light lights[8];
uniform vec3 eyePos;
uniform float globalAmbience;
uniform vec3 directionalLightDir;
uniform vec3 directionalLightColor;
uniform sampler2DArray textureArray;

flat in uint textureIndex;
flat in uint Shininess;
in float Reflectivity;
in vec2 textureCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

vec3 calculateLight(Light light, vec3 normal, vec3 pos, vec3 eyeDir) {
    // Ambient lighting
    vec3 ambient = globalAmbience * light.color * light.intensity;

    // diffuse lighting
    vec3 lightDir = normalize(light.position - pos);
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * light.color * light.intensity;

    // Specular lighting
    vec3 halfwayDir = normalize(lightDir + eyeDir);
    vec3 specular = Reflectivity * (pow(max(dot(normal, halfwayDir), 0.0), Shininess) * light.color * light.intensity);

    // Attenuation
    float distance = length(light.position - pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    // Apply attenuation
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main() {
    // Texture color
    vec3 textureColor = vec3(texture(textureArray, vec3(textureCoord, textureIndex)));
    
    // Directional variables
    vec3 normal = normalize(Normal);
    vec3 eyeDir = normalize(eyePos - FragPos);

    // Add directional light (sun/moon)
    vec3 lightDir = normalize(-directionalLightDir); // Sunlight direction
    // ambient (the directional light has twice the ambience)
    vec3 ambient = 2.0 * globalAmbience * directionalLightColor;
    // diffuse
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * directionalLightColor;
    // Specular
    vec3 halfwayDir = normalize(lightDir + eyeDir);
    vec3 specular = (Reflectivity * pow(max(dot(normal, halfwayDir), 0.0), Shininess)) * directionalLightColor;

    vec3 result = ambient + diffuse + specular;    
    
    // Add point lights
    for (int i = 0; i < numLights; ++i) {
        result += calculateLight(lights[i], normal, FragPos, eyeDir);
    }

    // Combine results
    FragColor = vec4(result * textureColor, 1.0);
}