#shader vertex
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in uint materialID;
layout (location = 3) in uint AOvalue;

out uint matID;
out float AO;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 matCamera;

void main() {
    gl_Position = matCamera * vec4(position, 1.0);

    // Transform normal and frag position to world space
    FragPos = position;
    Normal = normal;

    matID = materialID;
    AO = float(AOvalue);
}


#shader fragment
#version 460 core

struct Material {
    vec3 color;
    float reflectivity;
    uint shininess;
};

uniform vec3 eyePos;
uniform float globalAmbience;
uniform vec3 directionalLightDir;
uniform vec3 directionalLightColor;
uniform Material materials[256];

in vec3 Normal;
in vec3 FragPos;
flat in uint matID;
in float AO;

out vec4 fragColor;

void main() {
    // Directional variables
    vec3 normal = normalize(Normal);
    vec3 eyeDir = normalize(eyePos - FragPos);

    // Add directional light (sun/moon)
    vec3 lightDir = normalize(-directionalLightDir); // Sunlight direction
    
    // ambient
    vec3 ambient = globalAmbience * directionalLightColor * AO;
    
    // diffuse
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * directionalLightColor;
    
    // Specular
    vec3 specular = vec3(0, 0, 0);
    if (materials[matID].reflectivity != 0.0) {
        vec3 halfwayDir = normalize(lightDir + eyeDir);
        specular = materials[matID].reflectivity * (pow(max(dot(normal, halfwayDir), 0.0), materials[matID].shininess)) * directionalLightColor;
    } 

    vec3 result = ambient + diffuse + specular;    

    fragColor = vec4(materials[matID].color * result, 1.0);
}