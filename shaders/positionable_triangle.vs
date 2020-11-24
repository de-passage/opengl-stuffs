#version 330 core

layout (location = 0) in vec2 vertPos;

uniform vec2 xyOffset; 

void main() {
    gl_Position = vec4(xyOffset + vertPos, 1.0, 1.0);
}