#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

// _texture is a required uniform
uniform sampler2D _texture;

uniform float blurRadius;

out vec4 fragColor;

void main() {
    vec4 sumColor = vec4(0.0);
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