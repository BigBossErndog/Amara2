#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(_texture, texCoord);

    if (texColor.a > 0.0) {
        fragColor = texColor * fragTint;
        fragColor.a *= fragAlpha;
    } else {
        vec2 texelSize = 1.0 / vec2(textureSize(_texture, 0));
        float outline = 0.0;

        // Check all 8 surrounding pixels
        outline += texture(_texture, texCoord + vec2(-texelSize.x, 0.0)).a;
        outline += texture(_texture, texCoord + vec2(texelSize.x, 0.0)).a;
        outline += texture(_texture, texCoord + vec2(0.0, -texelSize.y)).a;
        outline += texture(_texture, texCoord + vec2(0.0, texelSize.y)).a;

        outline += texture(_texture, texCoord + vec2(-texelSize.x, -texelSize.y)).a;
        outline += texture(_texture, texCoord + vec2(texelSize.x, -texelSize.y)).a;
        outline += texture(_texture, texCoord + vec2(-texelSize.x, texelSize.y)).a;
        outline += texture(_texture, texCoord + vec2(texelSize.x, texelSize.y)).a;

        if (outline > 0.0) {
            fragColor = vec4(0.0, 0.0, 0.0, fragAlpha);
        } else {
            discard;
        }
    }
}