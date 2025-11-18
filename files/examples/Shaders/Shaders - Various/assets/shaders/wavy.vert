#version 330 core

layout (location = 0) in vec2 _position;
layout (location = 1) in vec2 _texCoord;
layout (location = 2) in float _alpha;
layout (location = 3) in vec4 _tint;

uniform float _time;

out vec2 texCoord;
out float fragAlpha;
out vec4 fragTint;

void main() {
    float scale = 1.05;
    vec2 scaledPos = _position * scale;

    float wave1 = sin(scaledPos.x * 6.0 + _time * 1.5) * 0.03;
    float wave2 = cos(scaledPos.y * 4.0 + _time * 2.0) * 0.02;
    float wave3 = sin((scaledPos.x + scaledPos.y) * 3.5 + _time * 1.2) * 0.015;

    float displacement = wave1 + wave2 + wave3;
    
    vec2 wavyPos = scaledPos + vec2(0.0, displacement);

    gl_Position = vec4(wavyPos, 0.0, 1.0);
    texCoord = _texCoord;
    fragAlpha = _alpha;
    fragTint = _tint;
}
