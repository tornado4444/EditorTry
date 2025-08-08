#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform vec3 camPos;
uniform float time;

out vec3 FragPos;
out float DistanceToCamera;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    DistanceToCamera = length(camPos - FragPos);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}