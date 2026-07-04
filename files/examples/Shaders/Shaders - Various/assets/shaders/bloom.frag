in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

// --- Bloom Effect Controls ---

// Threshold for what is considered "bright" enough to bloom.
// Range: 0.0 (everything blooms) to 1.0 (only brightest parts bloom).
// A good starting value is 0.7.

const float bloomThreshold = 0.65;

// Radius of the blur in pixels.
// Larger values create a softer, more spread-out bloom, but are much slower.
// Keep this value small. 1 or 2 is recommended for real-time performance.
// This is an integer, but GLSL requires it to be a float in the loop.

const float blurRadius = 2.0; // Results in a 5x5 kernel (2*2+1)

void main() {
    vec2 pixelSize = 1.0 / textureSize(_texture, 0);
    vec4 originalColor = texture(_texture, texCoord);

    vec4 bloomSum = vec4(0.0);
    float sampleCount = 0.0;

    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
        for (float y = -blurRadius; y <= blurRadius; y += 1.0) {
            vec2 offset = vec2(x, y) * pixelSize;
            vec4 sampleColor = texture(_texture, texCoord + offset);
            float brightness = dot(sampleColor.rgb, vec3(0.2126, 0.7152, 0.0722));
            
            if (brightness > bloomThreshold) {
                bloomSum += sampleColor;
            }
            sampleCount += 1.0;
        }
    }
    
    vec4 blurredBrights = bloomSum / sampleCount;
    fragColor = originalColor + blurredBrights;
    fragColor *= fragTint;
    fragColor.a *= fragAlpha;
}
