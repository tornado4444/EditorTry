#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in float DistanceToCamera;

uniform vec3 camPos;
uniform float time;

void main(){
    vec3 glassColor = vec3(0.85, 0.92, 1.0);

    float depthFactor = 1.0 - (DistanceToCamera / 20.0);
    depthFactor = clamp(depthFactor, 0.3, 1.0);
    
    vec3 finalColor = glassColor * depthFactor;

    finalColor += vec3(sin(time * 2.0) * 0.05);

    float alpha = 0.3 + (1.0 - depthFactor) * 0.2;
    
    FragColor = vec4(finalColor, alpha);
}