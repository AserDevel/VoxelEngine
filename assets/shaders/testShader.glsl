#shader vertex
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in uint shininess;
layout (location = 4) in float reflectivity;
layout (location = 5) in mat4 matWorld;

uniform mat4 matCamera;

out vec3 Normal;
out vec3 FragPos;
out vec3 FragColor;
out uint Shininess;
out float Reflectivity;

void main() {
    gl_Position = matCamera * matWorld * vec4(position, 1.0);
    
    // Transform normal and frag position to world space
    FragPos = vec3(matWorld * vec4(position, 1.0));
    Normal = vec3(transpose(inverse(matWorld)) * vec4(normal, 0.0));

    FragColor = color;
    Shininess = shininess;
    Reflectivity = reflectivity;
}



#shader fragment
#version 460 core

uniform vec3 eyePos;
uniform float globalAmbience;
uniform vec3 directionalLightDir;
uniform vec3 directionalLightColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 FragColor;
flat in uint Shininess;
in float Reflectivity;

out vec4 fragColor;

void main() {
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

    fragColor = vec4(1,1,1, 1.0);
}