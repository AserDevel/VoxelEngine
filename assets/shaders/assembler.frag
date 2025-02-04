#version 460 core

layout(location = 0) out vec4 fragColor;

uniform vec2 screenSize;
uniform isampler2D voxelTex;
uniform sampler2D lightingTex;

struct Material {
    vec4 color;
    int isTransparent;
    int isReflective;
};

uniform Material[256] materials;

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    uint voxel = texture(voxelTex, uv).r;

    vec3 color = materials[voxel].color.rgb;
    vec3 lighting = texture(lightingTex, uv).rgb;

    fragColor = vec4(color * lighting, 1.0);
}
