#version 460 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 FragPos;
    float DistanceToCamera;
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
} gs_in[];

uniform float time;

void main() {
    float normalLength = 0.1; 
    
    for(int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        
        vec4 normalEnd = gl_in[i].gl_Position + vec4(gs_in[i].Normal * normalLength, 0.0);
        gl_Position = normalEnd;
        EmitVertex();
        
        EndPrimitive();
    }
}
