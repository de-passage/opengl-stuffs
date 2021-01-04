#version 330 core

uniform vec3 light_color;
uniform vec3 object_color;
uniform float ambient;

out vec4 frag_color;

void main() {
    frag_color = vec4((light_color * ambient) * object_color, 1.0);
}