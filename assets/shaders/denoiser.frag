#version 460 core

layout(location = 0) out vec4 globalLight;

uniform bool cameraMoved;
uniform vec2 screenSize;
uniform sampler2D currFrame;
uniform sampler2D prevFrame;
uniform sampler2D positionTex;
uniform sampler2D normalTex;

/*
vec2 computeReprojected(vec3 position) {
    // World position from current frame
    vec4 worldPos = vec4(position, 1.0);

    // Convert back to previous clip space
    vec4 prevClipPos = prevViewProj * worldPos;

    // Perform perspective divide to normalize coordinates
    prevClipPos /= prevClipPos.w;

    // Convert NDC [-1, 1] back to UV [0, 1]
    vec2 uvReprojected = 0.5 + 0.5 * prevClipPos.xy;

    // Clamp to valid UV range
    uvReprojected = clamp(uvReprojected, vec2(0.0), vec2(1.0));

    return uvReprojected;
}
*/

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 position = texture(positionTex, uv).xyz;
    vec3 normal = texture(normalTex, uv).xyz;

    float mixFactor = 0.95;
    if (cameraMoved) {
        mixFactor = 0.2;
    }

    vec3 currLight = texture(currFrame, uv).rgb;
    vec3 prevLight = texture(prevFrame, uv).rgb;
    
    vec3 light = mix(currLight, prevLight, mixFactor);
    float totalWeight = 1.0;
    
    // sample neighbour pixels
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue;
            vec2 neighbourUV = (gl_FragCoord.xy + vec2(x, y)) / screenSize;
            vec3 neighbourPosition = texture(positionTex, neighbourUV).xyz;
            vec3 neighbourNormal = texture(normalTex, neighbourUV).xyz;
            if (normal == neighbourNormal && length(neighbourPosition - position) < 0.1) {
                // sample current and previous frame
                vec3 neighbourCurrLight = texture(currFrame, neighbourUV).rgb;
                vec3 neighbourPrevLight = texture(prevFrame, neighbourUV).rgb;

                // compute weight based on distance from the original pixel
                float weight = 1.0 / (abs(x) + abs(y));

                // add to sum, mix current and previous frame
                light += weight * mix(neighbourCurrLight, neighbourPrevLight, mixFactor);
                totalWeight += weight;
            }
        }
    }
    
    light /= totalWeight;

    globalLight = vec4(light, 1.0);
}
