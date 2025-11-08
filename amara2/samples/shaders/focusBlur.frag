#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

void main() {
    // Original color
    vec4 originalColor = texture(_texture, texCoord);

    // Blurred color (from box blur)
    vec4 sumColor = vec4(0.0);
    float blurRadius = 2.0; // For a 5x5 blur
    vec2 pixelSize = 1.0 / vec2(textureSize(_texture, 0)); // Size of one pixel in UV coordinates
    
    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
        for (float y = -blurRadius; y <= blurRadius; y += 1.0) {
            vec2 offset = vec2(x, y) * pixelSize;
            sumColor += texture(_texture, texCoord + offset);
        }
    }
    
    vec4 blurredColor = sumColor / ( (blurRadius * 2.0 + 1.0) * (blurRadius * 2.0 + 1.0) );

    // Calculate distance from center
    float dist = distance(texCoord, vec2(0.5, 0.5));
    
    // Adjust the start and end of the blur transition
    float blurStart = 0.2; // Start blurring at this distance from the center
    float blurEnd = 0.6;   // Full blur at this distance from the center
    
    // Smoothly interpolate the blur amount
    float blurAmount = smoothstep(blurStart, blurEnd, dist);

    // Blend between original and blurred color
    vec4 finalColor = mix(originalColor, blurredColor, blurAmount);

    // Apply tint and alpha from the vertex shader
    fragColor = finalColor * fragTint;
    fragColor.a *= fragAlpha;
}
