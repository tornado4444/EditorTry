#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in float DistanceToCamera;
in vec3 Normal;

uniform vec3 camPos;
uniform float time;
uniform float bunnySoftness;
uniform float geometryEffectStrength;

float saturate(float v) {
    return clamp(v, 0.0, 1.0);
}

void main() {
    float softness = saturate(bunnySoftness);

    vec3 n = normalize(Normal);
    vec3 v = normalize(camPos - FragPos);
    vec3 l = normalize(vec3(0.38, 0.88, 0.21));

    float ndl = max(dot(n, l), 0.0);
    float ndv = max(dot(n, v), 0.0);
    float hemi = n.y * 0.5 + 0.5;

    float rim = pow(1.0 - ndv, mix(4.5, 1.8, softness));
    float backScatter = pow(max(dot(-l, n), 0.0), 2.2);

    float fluffVariation = 1.0;

    vec3 base = mix(vec3(0.78, 0.84, 0.92), vec3(0.92, 0.96, 1.0), softness) * fluffVariation;
    vec3 ambient = mix(vec3(0.10, 0.13, 0.16), vec3(0.30, 0.36, 0.44), softness) * (0.55 + 0.45 * hemi);
    vec3 diffuse = base * ndl * mix(0.45, 0.85, softness);
    vec3 subsurface = vec3(0.78, 0.86, 0.98) * backScatter * mix(0.05, 0.22, softness);
    vec3 rimLight = vec3(0.95, 0.98, 1.0) * rim * mix(0.08, 0.35, softness);

    float depthFog = saturate(DistanceToCamera / 35.0);
    vec3 fogColor = vec3(0.42, 0.54, 0.72);

    vec3 finalColor = ambient + diffuse + subsurface + rimLight;

    if (geometryEffectStrength > 0.001) {
        vec3 quantized = floor(finalColor * 6.0) / 6.0;
        float contour = pow(1.0 - ndv, 1.6);
        float sweep = abs(fract(dot(FragPos, vec3(0.20, 0.33, 0.27)) + time * 0.65) - 0.5);
        float sweepLine = 1.0 - smoothstep(0.40, 0.50, sweep);

        vec3 geoLayer = quantized;
        geoLayer += vec3(0.15, 0.26, 0.40) * contour;
        geoLayer += vec3(0.12, 0.22, 0.34) * sweepLine;

        finalColor = mix(finalColor, geoLayer, geometryEffectStrength * 0.85);
    }

    finalColor = mix(finalColor, fogColor, depthFog * 0.12);
    finalColor = clamp(finalColor, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
