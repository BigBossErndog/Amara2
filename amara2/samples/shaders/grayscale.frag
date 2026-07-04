in vec2 texCoord;
in float fragAlpha;
in vec4 fragTint;

uniform sampler2D _texture;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(_texture, texCoord);
    
    float r = (texColor.r + texColor.g + texColor.b) / 3;
    float g = (texColor.r + texColor.g + texColor.b) / 3;
    float b = (texColor.r + texColor.g + texColor.b) / 3;
    vec3 grayscaleColor = vec3(r, g, b);

    fragColor = vec4(grayscaleColor, texColor.a) * fragTint;
    fragColor.a *= fragAlpha;
}