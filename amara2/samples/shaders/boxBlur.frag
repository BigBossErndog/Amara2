#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

void main() {
    vec4 sumColor = vec4(0.0);
    float blurRadius = 1.0; // For a 3x3 blur, radius is 1 pixel
    vec2 pixelSize = 1.0 / vec2(textureSize(_texture, 0)); // Size of one pixel in UV coordinates
    
    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
        for (float y = -blurRadius; y <= blurRadius; y += 1.0) {
            vec2 offset = vec2(x, y) * pixelSize;
            sumColor += texture(_texture, texCoord + offset);
        }
    }
    
    vec4 blurredColor = sumColor / ( (blurRadius * 2.0 + 1.0) * (blurRadius * 2.0 + 1.0) );

    // Apply tint and alpha from the vertex shader
    fragColor = blurredColor * fragTint;
    fragColor.a *= fragAlpha;
}