#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform vec3 camPos;
uniform float time;

out VS_OUT {
    vec3 FragPos;
    float DistanceToCamera;
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
} vs_out;

void main(){
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.DistanceToCamera = length(camPos - vs_out.FragPos);
    
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    
    vs_out.texCoord = aTexCoord;
    
    vs_out.color = vec3(1.0, 1.0, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}