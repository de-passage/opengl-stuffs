#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 colors;

out vec4 vertexColor;

uniform mat4 model;
uniform mat4 projected_view;

void main()
{
    gl_Position = projected_view * model * vec4(aPos, 1.0f);
    vertexColor = colors;
}