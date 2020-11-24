#version 330 core\n

out vec4 outputColor;\n
void main() {
    float lerpValue = gl_FragCoord.y / 500.0f;
    outputColor = mix(vec4(1.0f, 1.0f, 1.0f, 1.0f),
                        vec4(0.2f, 0.2f, 0.2f, 1.0f), lerpValue);
}