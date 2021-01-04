#version 330 core

in vec3 fragment_normal;
in vec3 fragment_position;

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 object_color;
uniform float ambient;

out vec4 frag_color;

void main() {
    vec3 ambient_lighting = ambient * light_color;

    vec3 norm = normalize(fragment_normal);
    vec3 light_dir = normalize(light_position - fragment_position);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse_lighting = diff * light_color;

    frag_color = vec4((ambient_lighting + diffuse_lighting) * object_color, 1.0);
}