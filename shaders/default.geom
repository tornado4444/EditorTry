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

void main() {
    for(int i = 0; i < 3; i++) {
        FragPos = gs_in[i].FragPos;
        DistanceToCamera = gs_in[i].DistanceToCamera;
        Normal = gs_in[i].Normal;
        color = gs_in[i].color;
        texCoord = gs_in[i].texCoord;

        gl_Position = gl_in[i].gl_Position;

        EmitVertex();
    }
    EndPrimitive();
}
