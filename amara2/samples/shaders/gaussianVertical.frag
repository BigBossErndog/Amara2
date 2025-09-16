#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

void main() {
    vec4 sum = vec4(0.0);
    vec2 oneTexel = 1.0 / vec2(textureSize(_texture, 0));

    // Gaussian weights for a 5-tap kernel (approximate, sum to 16)
    // [1, 4, 6, 4, 1]
    // Sum = 16

    // Sample vertically
    sum += texture(_texture, texCoord + vec2(0.0, -2.0 * oneTexel.y)) * 1.0;
    sum += texture(_texture, texCoord + vec2(0.0, -1.0 * oneTexel.y)) * 4.0;
    sum += texture(_texture, texCoord + vec2(0.0,  0.0 * oneTexel.y)) * 6.0;
    sum += texture(_texture, texCoord + vec2(0.0,  1.0 * oneTexel.y)) * 4.0;
    sum += texture(_texture, texCoord + vec2(0.0,  2.0 * oneTexel.y)) * 1.0;

    fragColor = (sum / 16.0) * fragTint;
    fragColor.a *= fragAlpha;
}