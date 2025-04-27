#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

// NEW: You need to add these uniforms for glow
uniform vec4 glowColor;      // color of the glow
uniform float glowRadius;    // how far the glow spreads (in pixels)

out vec4 fragColor;

void main() {
    vec4 texColor = texture(_texture, texCoord);

    // Base sprite color
    vec4 spriteColor = texColor * fragTint;
    spriteColor.a *= fragAlpha;
    
    // Calculate texel size based on texture size
    vec2 texSize = vec2(textureSize(_texture, 0));
    vec2 texelSize = 1.0 / texSize;

    // If inside the sprite, just render normally
    if (spriteColor.a > 0.0) {
        fragColor = spriteColor;
        return;
    }

    // Otherwise, check nearby pixels for alpha to create a glow
    float glowAmount = 0.0;
    
    for (float x = -glowRadius; x <= glowRadius; x++) {
        for (float y = -glowRadius; y <= glowRadius; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            vec4 sample = texture(_texture, texCoord + offset);
            glowAmount += sample.a;
        }
    }

    float totalSamples = (2.0 * glowRadius + 1.0) * (2.0 * glowRadius + 1.0);
    glowAmount /= totalSamples;

    // Final color: soft glow where sprite is transparent but near alpha
    fragColor = glowColor * glowAmount;
    fragColor.a *= fragAlpha; // Respect overall alpha
}
