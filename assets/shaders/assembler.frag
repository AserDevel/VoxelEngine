#version 460 core

layout(location = 0) out vec4 fragColor;

uniform vec2 screenSize;
uniform sampler2D colorTex;
uniform sampler2D globalLightTex;
uniform sampler2D specialLightTex;

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 color = texture(colorTex, uv).rgb;
    vec3 globalLight = texture(globalLightTex, uv).rgb;
    vec3 specialLight = texture(specialLightTex, uv).rgb;

    fragColor = vec4(color * globalLight, 1.0);
}
