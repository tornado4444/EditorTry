#version 460 core

out vec4 color;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec3 Color;

void main(){
	vec4 sampled = vec4(1.0f,1.0f,1.0f, texture(image,TexCoords).r);
	color = vec4(Color, 1.0f) * sampled;
}