#version 460 core
layout (location = 0) in vec3 aPos;      
layout (location = 1) in vec3 instCenter;
layout (location = 2) in vec3 instScale; 
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 scaledPos = aPos * instScale;
    vec3 worldPos = instCenter + scaledPos;
    
    worldPos.y = -worldPos.y; 
    worldPos.z = -worldPos.z;
    float angle = radians(90.0); 
    float c = cos(angle);
    float s = sin(angle);
    vec3 rotated;
    rotated.x = c * worldPos.x + s * worldPos.z;
    rotated.y = worldPos.y;
    rotated.z = -s * worldPos.x + c * worldPos.z;
    worldPos = rotated;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}