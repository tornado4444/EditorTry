#version 460 core
layout (location = 0) in vec3 aPos;      
layout (location = 1) in vec3 instCenter;
layout (location = 2) in vec3 instScale; 
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 scaledPos = aPos * instScale;
    vec3 worldPos = instCenter + scaledPos;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
