#version 330 core

in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(_texture, texCoord);
    
    float r = texColor.r * 0.393 + texColor.g * 0.769 + texColor.b * 0.189;
    float g = texColor.r * 0.349 + texColor.g * 0.686 + texColor.b * 0.168;
    float b = texColor.r * 0.272 + texColor.g * 0.534 + texColor.b * 0.131;
    vec3 sepiaColor = vec3(r, g, b);

    fragColor = vec4(sepiaColor, texColor.a) * fragTint;
    fragColor.a *= fragAlpha;
}