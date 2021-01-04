#version 330 core

in vec3 fragment_normal;
in vec3 fragment_position;

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 object_color;
uniform vec3 camera_position;
uniform float ambient;
uniform float specular;
uniform int shininess;

out vec4 frag_color;

void main() {
    // Ambient
    vec3 ambient_lighting = ambient * light_color;

    // Diffuse
    vec3 norm = normalize(fragment_normal);
    vec3 light_dir = normalize(light_position - fragment_position);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse_lighting = diff * light_color;

    // specular
    vec3 view_dir = normalize(camera_position - fragment_position);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular_lighting = specular * spec * light_color;

    frag_color = vec4((ambient_lighting + diffuse_lighting + specular_lighting) * object_color, 1.0);
}