#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 projected_view;

out vec3 fragment_position;
out vec3 fragment_normal;

void main() {
    fragment_position = vec3(model * vec4(position, 1.0));
    fragment_normal = normal;
    gl_Position = projected_view * model * vec4(position, 1.0);
}