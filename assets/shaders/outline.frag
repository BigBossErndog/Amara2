#version 330 core

in vec2 texCoord;
in float fragAlpha; // Overall alpha for the sprite
in vec4 fragTint;   // Tint color for the sprite

uniform sampler2D _texture;
uniform vec2 textureSize; // Texture dimensions (width, height) in pixels

out vec4 fragColor;

void main() {
    vec2 texel = 1.0 / textureSize; // Size of one pixel in texture coordinates
    float alphaThreshold = 0.1; // Minimum alpha to be considered 'solid'

    // Sample the center pixel
    vec4 centerColor = texture(_texture, texCoord);
    float centerAlpha = centerColor.a;

    // Sample neighbors (you can add diagonals for a thicker outline)
    float maxNeighborAlpha = 0.0;
    maxNeighborAlpha = max(maxNeighborAlpha, texture(_texture, texCoord + vec2(texel.x, 0.0)).a);  // Right
    maxNeighborAlpha = max(maxNeighborAlpha, texture(_texture, texCoord - vec2(texel.x, 0.0)).a);  // Left
    maxNeighborAlpha = max(maxNeighborAlpha, texture(_texture, texCoord + vec2(0.0, texel.y)).a);  // Up (Texture Y might be inverted)
    maxNeighborAlpha = max(maxNeighborAlpha, texture(_texture, texCoord - vec2(0.0, texel.y)).a);  // Down

    // Check if the current pixel is transparent but a neighbor is not
    if (centerAlpha <= alphaThreshold && maxNeighborAlpha > alphaThreshold) {
        // Draw outline pixel (black)
        // Use the max neighbor alpha modulated by the overall sprite alpha
        fragColor = vec4(0.0, 0.0, 0.0, maxNeighborAlpha * fragAlpha);
    } else {
        // Draw original sprite pixel (or fully transparent background)
        fragColor = centerColor * fragTint; // Apply tint
        fragColor.a *= fragAlpha;           // Apply overall alpha
    }
}