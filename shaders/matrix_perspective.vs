#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;

out vec4 vertexColor;

uniform mat4 perspective;
uniform vec2 offset;

void main() {
    vec4 camera_pos = position + vec4(offset, 0, 0);

    gl_Position = perspective * camera_pos;
    vertexColor = color;
}