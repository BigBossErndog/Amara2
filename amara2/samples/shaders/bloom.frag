in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

// --- Bloom Effect Controls ---

// Threshold for what is considered "bright" enough to bloom.
// Range: 0.0 (everything blooms) to 1.0 (only brightest parts bloom).
// A good starting value is 0.7.
const float bloomThreshold = 0.7;

// Radius of the blur in pixels.
// Larger values create a softer, more spread-out bloom, but are much slower.
// Keep this value small. 1 or 2 is recommended for real-time performance.
// This is an integer, but GLSL requires it to be a float in the loop.
const float blurRadius = 2.0; // Results in a 5x5 kernel (2*2+1)

void main() {
    vec2 pixelSize = 1.0 / textureSize(_texture, 0); // Size of one pixel in UV coordinates
    vec4 originalColor = texture(_texture, texCoord);

    vec4 bloomSum = vec4(0.0);
    float sampleCount = 0.0;

    // Iterate over a square area around the current pixel
    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
        for (float y = -blurRadius; y <= blurRadius; y += 1.0) {
            vec2 offset = vec2(x, y) * pixelSize;
            vec4 sampleColor = texture(_texture, texCoord + offset);
            
            // Calculate the brightness of the sampled pixel
            // Using luminance calculation: dot(color.rgb, vec3(0.2126, 0.7152, 0.0722))
            float brightness = dot(sampleColor.rgb, vec3(0.2126, 0.7152, 0.0722));
            
            // If the brightness is above the threshold, add it to our sum
            if (brightness > bloomThreshold) {
                bloomSum += sampleColor;
            }
            sampleCount += 1.0;
        }
    }
    
    // Average the collected bright colors
    vec4 blurredBrights = bloomSum / sampleCount;

    // Add the blurred bright colors to the original color (additive blending)
    // The intensity of the bloom can be controlled by multiplying blurredBrights
    // e.g., originalColor + blurredBrights * 0.5;
    fragColor = original-Color + blurredBrights;

    // Apply tint and alpha from the vertex shader
    fragColor *= fragTint;
    fragColor.a *= fragAlpha;
}
