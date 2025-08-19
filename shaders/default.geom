#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 FragPos;
    float DistanceToCamera;
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
} gs_in[];

out vec3 FragPos;
out float DistanceToCamera;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

uniform float time;

void main() {
    for(int i = 0; i < 3; i++) {
        FragPos = gs_in[i].FragPos;
        DistanceToCamera = gs_in[i].DistanceToCamera;
        Normal = gs_in[i].Normal;
        color = gs_in[i].color;
        texCoord = gs_in[i].texCoord;
        
        vec4 pos = gl_in[i].gl_Position;
        pos.xyz += Normal * sin(time + pos.x * 0.5) * 0.01;
        gl_Position = pos;
        
        EmitVertex();
    }
    EndPrimitive();
}