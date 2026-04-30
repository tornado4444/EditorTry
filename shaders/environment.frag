#version 460 core

out vec4 FragColor;
in vec2 vUv;

uniform float Time;
uniform float screenWidth;
uniform float screenHeight;

uniform vec3 cameraPosition;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform vec3 EarthCenter;
uniform float CloudBottom;
uniform float CloudTop;
uniform float CloudDensity;
uniform float CloudCoverage;
uniform float CloudSoftness;
uniform float CloudStorminess;
uniform float OceanWaveStrength;
uniform float UnderwaterDensity;
uniform float OceanWireframe;

uniform float SkyTimeHours;
uniform float StarBrightness;
uniform float MoonBrightness;
uniform float MoonSizeDegrees;
uniform int UseMoonTexture;
uniform int UseStarTexture;

uniform sampler3D lowFrequencyTexture;
uniform sampler3D highFrequencyTexture;
uniform sampler2D WeatherTexture;
uniform sampler2D CurlNoiseTexture;
uniform sampler2D MoonTexture;
uniform sampler2D StarTexture;

const float PI = 3.14159265359;
const float EARTH_RADIUS = 6378000.0;

float saturate(float v) {
    return clamp(v, 0.0, 1.0);
}

float dayFactorFromSun(vec3 sunDir) {
    return saturate((sunDir.y + 0.08) / 0.38);
}

float nightFactorFromSun(vec3 sunDir) {
    return saturate((-sunDir.y + 0.05) / 0.55);
}

float hash13(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.zyx + 31.32);
    return fract((p.x + p.y) * p.z);
}

float hash21(vec2 p) {
    vec3 p3 = fract(vec3(p.x, p.y, p.x) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise2D(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    float a = hash21(i + vec2(0.0, 0.0));
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p) {
    float f = 0.0;
    float a = 0.5;
    for (int i = 0; i < 5; ++i) {
        f += a * noise2D(p);
        p *= 2.03;
        a *= 0.5;
    }
    return f;
}

vec2 dirToEquirectUV(vec3 d) {
    d = normalize(d);
    float u = atan(d.z, d.x) / (2.0 * PI) + 0.5;
    float v = asin(clamp(d.y, -1.0, 1.0)) / PI + 0.5;
    return vec2(fract(u), clamp(v, 0.0, 1.0));
}

vec3 computeSunDirection(float skyHours) {
    float angle = (skyHours / 24.0) * 2.0 * PI - 0.5 * PI;
    return normalize(vec3(
        cos(angle),
        sin(angle),
        0.32 * sin(angle * 0.41 + 0.8)
    ));
}

vec3 computeMoonDirection(float skyHours) {
    float angle = (skyHours / 24.0) * 2.0 * PI - 0.5 * PI + PI + 0.45;
    return normalize(vec3(
        cos(angle),
        sin(angle + 0.08),
        0.36 * sin(angle * 0.38 - 0.3)
    ));
}

float oceanWireMask(vec2 xz, float distanceToHit) {
    vec2 fine = abs(fract(xz * 0.22) - 0.5);
    vec2 coarse = abs(fract(xz * 0.055) - 0.5);

    float fineWidth = mix(0.02, 0.07, saturate(distanceToHit / 2600.0));
    float coarseWidth = fineWidth * 1.5;

    float fineLine = 1.0 - smoothstep(fineWidth, fineWidth * 1.9, min(fine.x, fine.y));
    float coarseLine = 1.0 - smoothstep(coarseWidth, coarseWidth * 1.9, min(coarse.x, coarse.y));

    float fade = exp(-distanceToHit * 0.0012);
    return saturate(max(fineLine * 0.75, coarseLine * 0.55) * fade);
}

vec3 tonemap(vec3 x) {
    return x / (1.0 + x);
}

float proceduralStars(vec3 rd) {
    vec2 uv = dirToEquirectUV(rd);
    vec2 grid = uv * vec2(7000.0, 3500.0);
    vec2 id = floor(grid);
    vec2 f = fract(grid) - 0.5;

    float rnd = hash21(id);
    float starMask = smoothstep(0.9982, 0.99998, rnd);
    float core = exp(-dot(f, f) * 90.0);
    float twinkle = 0.65 + 0.35 * sin(Time * 3.2 + rnd * 120.0);

    return starMask * core * twinkle;
}

vec3 sampleStarField(vec3 rd, float nightFactor) {
    float horizonMask = smoothstep(-0.06, 0.28, rd.y);
    if (horizonMask <= 0.0 || nightFactor <= 0.0) {
        return vec3(0.0);
    }

    vec3 nasaStars = vec3(0.0);
    if (UseStarTexture > 0) {
        vec2 uv = dirToEquirectUV(rd);
        uv.x = fract(uv.x + Time * 0.00095);
        nasaStars = texture(StarTexture, uv).rgb;
        float lum = dot(nasaStars, vec3(0.2126, 0.7152, 0.0722));
        nasaStars = pow(max(nasaStars, vec3(0.0)), vec3(1.35));
        nasaStars *= smoothstep(0.02, 0.45, lum);
    }

    float pStars = proceduralStars(rd);
    vec3 proceduralCol = vec3(0.78, 0.86, 1.0) * pStars;

    vec3 stars = nasaStars * 1.45 + proceduralCol * 1.25;
    stars *= StarBrightness * nightFactor * horizonMask;
    return stars;
}

vec3 sampleMoonDisk(vec3 rd, vec3 moonDir, vec3 sunDir, float nightFactor, out float moonMask, out float moonHalo) {
    float moonRadius = radians(clamp(MoonSizeDegrees, 0.12, 4.0)) * 0.5;
    float moonCosRadius = cos(moonRadius);
    float moonDot = dot(rd, moonDir);
    float aa = max(fwidth(moonDot) * 1.5, 0.0003);
    moonMask = smoothstep(moonCosRadius - aa, moonCosRadius + aa, moonDot);

    moonHalo = exp(-(1.0 - moonDot) * (160.0 / max(moonRadius, 0.001)));
    moonHalo *= nightFactor;

    if (moonMask <= 0.0) {
        return vec3(0.0);
    }

    vec3 refUp = abs(moonDir.y) > 0.98 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 moonRight = normalize(cross(refUp, moonDir));
    vec3 moonUp = normalize(cross(moonDir, moonRight));

    vec3 local = normalize(vec3(dot(rd, moonRight), dot(rd, moonUp), dot(rd, moonDir)));
    vec2 moonUv = vec2(
        atan(local.x, local.z) / (2.0 * PI) + 0.5,
        asin(clamp(local.y, -1.0, 1.0)) / PI + 0.5
    );

    vec3 moonAlbedo = vec3(0.76, 0.76, 0.79);
    if (UseMoonTexture > 0) {
        vec2 texUvEq = vec2(fract(moonUv.x), 1.0 - clamp(moonUv.y, 0.0, 1.0));
        vec2 texUvDisc = vec2(local.x, local.y) * 0.5 + 0.5;

        float centerLum = dot(texture(MoonTexture, vec2(0.5, 0.5)).rgb, vec3(0.2126, 0.7152, 0.0722));
        float cornerLum =
            dot(texture(MoonTexture, vec2(0.02, 0.02)).rgb, vec3(0.2126, 0.7152, 0.0722)) +
            dot(texture(MoonTexture, vec2(0.98, 0.02)).rgb, vec3(0.2126, 0.7152, 0.0722)) +
            dot(texture(MoonTexture, vec2(0.02, 0.98)).rgb, vec3(0.2126, 0.7152, 0.0722)) +
            dot(texture(MoonTexture, vec2(0.98, 0.98)).rgb, vec3(0.2126, 0.7152, 0.0722));
        cornerLum *= 0.25;

        float discLike = step(cornerLum * 3.0, centerLum);
        vec3 texEq = texture(MoonTexture, texUvEq).rgb;
        vec3 texDisc = texture(MoonTexture, texUvDisc).rgb;
        moonAlbedo = pow(mix(texEq, texDisc, discLike), vec3(1.08));
    }

    vec3 moonPointNormal = normalize(moonRight * local.x + moonUp * local.y + moonDir * local.z);
    float lambert = max(dot(moonPointNormal, -sunDir), 0.0);
    float phase = saturate(0.5 + 0.5 * dot(moonDir, -sunDir));
    float earthshine = mix(0.11, 0.04, phase);
    float lit = earthshine + lambert * mix(0.45, 1.35, phase);
    float visibility = mix(0.55, 1.0, nightFactor);
    float rim = pow(1.0 - saturate(local.z), 3.0) * 0.18;

    return moonAlbedo * (lit + rim) * MoonBrightness * visibility * moonMask;
}

vec3 skyColor(vec3 rd, vec3 sunDir, vec3 moonDir) {
    float dayFactor = dayFactorFromSun(sunDir);
    float nightFactor = nightFactorFromSun(sunDir);
    float t = saturate(rd.y * 0.5 + 0.5);

    vec3 daySky = mix(vec3(0.66, 0.74, 0.86), vec3(0.20, 0.48, 0.86), pow(t, 0.65));
    vec3 nightSky = mix(vec3(0.03, 0.05, 0.09), vec3(0.006, 0.014, 0.034), pow(t, 0.85));
    vec3 base = mix(nightSky, daySky, dayFactor);

    float sunset = exp(-abs(sunDir.y) * 7.5) * smoothstep(-0.1, 0.42, sunDir.y + 0.12);
    float horizon = pow(1.0 - saturate(abs(rd.y)), 2.4);
    base += vec3(0.95, 0.36, 0.14) * horizon * sunset * 0.42;

    float sunDot = max(dot(rd, sunDir), 0.0);
    float sunHalo = exp(-(1.0 - sunDot) * 180.0);
    float sunGlow = pow(sunDot, 34.0);
    float sunDisk = pow(sunDot, 900.0);
    base += vec3(1.0, 0.76, 0.41) * sunHalo * mix(0.10, 1.15, dayFactor);
    base += vec3(1.0, 0.88, 0.62) * sunGlow * mix(0.28, 1.25, dayFactor);
    base += vec3(1.0, 0.97, 0.88) * sunDisk * 11.0 * dayFactor;

    float moonMask;
    float moonHalo;
    vec3 moonCol = sampleMoonDisk(rd, moonDir, sunDir, nightFactor, moonMask, moonHalo);
    base += moonCol;
    base += vec3(0.48, 0.58, 0.76) * moonHalo * MoonBrightness * 0.22;

    base += sampleStarField(rd, nightFactor);
    return base;
}

void waveSpectrum(vec2 xz, float t, float strength, out float h, out vec2 grad, out float crest) {
    vec2 d1 = normalize(vec2(0.86, 0.51));
    vec2 d2 = normalize(vec2(-0.63, 0.78));
    vec2 d3 = normalize(vec2(0.24, -0.97));
    vec2 d4 = normalize(vec2(0.94, -0.35));
    vec2 d5 = normalize(vec2(-0.18, -0.98));

    float A1 = 1.05 * strength;
    float A2 = 0.72 * strength;
    float A3 = 0.44 * strength;
    float A4 = 0.27 * strength;
    float A5 = 0.18 * strength;

    float L1 = 260.0;
    float L2 = 155.0;
    float L3 = 82.0;
    float L4 = 48.0;
    float L5 = 30.0;

    float k1 = 2.0 * PI / L1;
    float k2 = 2.0 * PI / L2;
    float k3 = 2.0 * PI / L3;
    float k4 = 2.0 * PI / L4;
    float k5 = 2.0 * PI / L5;

    float p1 = k1 * dot(d1, xz) + t * 0.58;
    float p2 = k2 * dot(d2, xz) + t * 0.94;
    float p3 = k3 * dot(d3, xz) + t * 1.38;
    float p4 = k4 * dot(d4, xz) + t * 1.86;
    float p5 = k5 * dot(d5, xz) + t * 2.35;

    float s1 = sin(p1), c1 = cos(p1);
    float s2 = sin(p2), c2 = cos(p2);
    float s3 = sin(p3), c3 = cos(p3);
    float s4 = sin(p4), c4 = cos(p4);
    float s5 = sin(p5), c5 = cos(p5);

    h = A1 * s1 + A2 * s2 + A3 * s3 + A4 * s4 + A5 * s5;

    grad = vec2(0.0);
    grad += A1 * k1 * d1 * c1;
    grad += A2 * k2 * d2 * c2;
    grad += A3 * k3 * d3 * c3;
    grad += A4 * k4 * d4 * c4;
    grad += A5 * k5 * d5 * c5;

    float chop = fbm(xz * 0.014 + vec2(t * 0.12, -t * 0.10));
    h += (chop - 0.5) * 0.42 * strength;

    float sharp = 0.0;
    sharp += pow(max(s1, 0.0), 4.0) * A1;
    sharp += pow(max(s2, 0.0), 4.0) * A2;
    sharp += pow(max(s3, 0.0), 4.0) * A3;
    crest = sharp;
}

bool intersectWaterSurface(vec3 ro, vec3 rd, float t, float strength, out float tHit, out vec3 hitPos, out vec3 hitNormal, out float crest) {
    if (abs(rd.y) < 1e-5) {
        return false;
    }

    float tPlane = (0.0 - ro.y) / rd.y;
    if (tPlane <= 0.0) {
        return false;
    }

    float tCur = tPlane;
    for (int i = 0; i < 7; ++i) {
        vec3 p = ro + rd * tCur;
        float h;
        vec2 grad;
        float crestDummy;
        waveSpectrum(p.xz, t, strength, h, grad, crestDummy);

        float f = p.y - h;
        float df = rd.y - dot(grad, rd.xz);
        float safeDf = (abs(df) < 0.035) ? (df < 0.0 ? -0.035 : 0.035) : df;
        tCur -= f / safeDf;
        tCur = max(tCur, 0.0);
    }

    if (tCur <= 0.0 || tCur > 220000.0) {
        return false;
    }

    hitPos = ro + rd * tCur;
    float h;
    vec2 grad;
    waveSpectrum(hitPos.xz, t, strength, h, grad, crest);
    hitPos.y = h;
    hitNormal = normalize(vec3(-grad.x, 1.0, -grad.y));

    float residual = abs((ro.y + rd.y * tCur) - h);
    if (residual > 3.0) {
        return false;
    }

    tHit = tCur;
    return true;
}

vec3 renderOceanAbove(vec3 ro, vec3 rd, float t, float strength, float underwaterDensity, vec3 sunDir, vec3 moonDir) {
    vec3 hitPos;
    vec3 n;
    float tHit;
    float crest;
    if (!intersectWaterSurface(ro, rd, t, strength, tHit, hitPos, n, crest)) {
        return skyColor(rd, sunDir, moonDir);
    }

    float dayFactor = dayFactorFromSun(sunDir);
    vec3 mainLightDir = normalize(mix(moonDir, sunDir, dayFactor));

    vec3 reflected = skyColor(reflect(rd, n), sunDir, moonDir);

    float ndotv = saturate(dot(n, normalize(-rd)));
    float fresnel = 0.02 + (1.0 - 0.02) * pow(1.0 - ndotv, 5.0);

    vec3 absorb = vec3(0.16, 0.07, 0.03) * max(underwaterDensity, 0.25);
    float opticalDepth = tHit * (0.018 + 0.060 * (1.0 - ndotv));
    vec3 trans = exp(-absorb * opticalDepth);
    vec3 refractedCol = mix(vec3(0.06, 0.36, 0.46), vec3(0.005, 0.07, 0.12), saturate(opticalDepth * 0.22));
    refractedCol *= (1.0 - trans * 0.45);

    float slope = length(n.xz) / max(n.y, 1e-4);
    float foam = smoothstep(0.38, 0.85, slope) + crest * 0.26;
    foam = saturate(foam);
    vec3 foamCol = vec3(0.83, 0.89, 0.93);

    float specPow = mix(38.0, 96.0, dayFactor);
    float sunSpec = pow(max(dot(reflect(-mainLightDir, n), -rd), 0.0), specPow);
    vec3 specColor = mix(vec3(0.45, 0.55, 0.74), vec3(1.0, 0.95, 0.75), dayFactor);
    vec3 specCol = specColor * sunSpec * mix(0.5, 1.35, dayFactor);

    float micro = fbm(hitPos.xz * 0.09 + vec2(Time * 0.9, -Time * 0.7));
    float sparkleMask = pow(saturate(1.0 - abs(micro - 0.5) * 2.0), 10.0);
    float sparkle = sparkleMask * saturate(0.2 + crest * 0.3) * pow(max(dot(reflect(-mainLightDir, n), -rd), 0.0), 32.0);

    vec3 col = mix(refractedCol, reflected, fresnel) + specCol;
    col += vec3(0.65, 0.75, 0.82) * sparkle * mix(0.07, 0.15, dayFactor);
    col = mix(col, foamCol, foam * 0.42);

    float horizonHaze = saturate(exp(-abs(rd.y) * 2.1));
    col = mix(col, vec3(0.54, 0.63, 0.72), horizonHaze * 0.16);

    if (OceanWireframe > 0.5) {
        float wire = oceanWireMask(hitPos.xz, tHit);
        vec3 wireColor = vec3(0.62, 0.93, 1.0);
        col = mix(col, wireColor, wire * 0.82);
    }

    return clamp(max(col, vec3(0.01, 0.03, 0.05)), 0.0, 1.0);
}

vec3 renderUnderwater(vec3 ro, vec3 rd, float t, float strength, float density, vec3 sunDir, vec3 moonDir) {
    float dayFactor = dayFactorFromSun(sunDir);
    vec3 mainLightDir = normalize(mix(moonDir, sunDir, dayFactor));

    vec3 hitPos;
    vec3 n;
    float tHit;
    float crest;
    bool hitSurface = rd.y > 0.0005 && intersectWaterSurface(ro, rd, t, strength, tHit, hitPos, n, crest);

    float travel = hitSurface ? min(tHit, 220.0) : 220.0;
    vec3 absorb = vec3(0.18, 0.075, 0.04) * max(density, 0.2);
    vec3 trans = exp(-absorb * travel * 0.50);

    vec3 scatterBase = mix(vec3(0.020, 0.10, 0.18), vec3(0.07, 0.33, 0.42), saturate(dayFactor * 0.9 + 0.1));
    vec3 col = scatterBase * (1.0 - trans);

    float sunForward = pow(max(dot(normalize(rd), mainLightDir), 0.0), mix(18.0, 30.0, dayFactor));
    float shaftMask = fbm((ro.xz + rd.xz * min(travel, 120.0)) * 0.024 + vec2(Time * 0.14, -Time * 0.11));
    shaftMask = smoothstep(0.45, 0.95, shaftMask);
    float shafts = sunForward * shaftMask * exp(-travel * 0.025 * density);
    vec3 shaftCol = mix(vec3(0.11, 0.16, 0.22), vec3(0.16, 0.28, 0.34), dayFactor);
    col += shaftCol * shafts;

    vec2 driftUv = (ro.xz + rd.xz * min(travel, 90.0)) * 0.055;
    float particulate = fbm(driftUv + vec2(Time * 0.34, -Time * 0.27));
    float sparkles = smoothstep(0.70, 0.98, particulate);
    col += vec3(0.05, 0.08, 0.09) * sparkles * (1.0 - trans.r) * 0.6;

    if (hitSurface) {
        vec3 upN = dot(n, vec3(0.0, 1.0, 0.0)) < 0.0 ? -n : n;
        float cosi = saturate(dot(-rd, upN));
        float eta = 1.333;
        float f0 = 0.02037;
        float fresnel = f0 + (1.0 - f0) * pow(1.0 - cosi, 5.0);

        vec3 reflectedSky = skyColor(reflect(rd, upN), sunDir, moonDir);
        vec3 refrDir = refract(rd, upN, eta);
        vec3 transmittedSky = length(refrDir) > 1e-4 ? skyColor(normalize(refrDir), sunDir, moonDir) : reflectedSky;
        vec3 surfaceCol = mix(transmittedSky, reflectedSky, fresnel);

        vec3 surfaceAtten = exp(-absorb * tHit * 0.44);
        col = mix(col, surfaceCol * surfaceAtten + col * 0.42, 0.72);

        vec2 causticUv = (hitPos.xz + rd.xz * 12.0) * 0.11;
        float caustic1 = fbm(causticUv + vec2(Time * 1.35, -Time * 1.10));
        float caustic2 = fbm(causticUv * 1.8 + vec2(-Time * 0.95, Time * 1.08));
        float caustic = smoothstep(0.56, 0.96, caustic1 * 0.6 + caustic2 * 0.4);
        vec3 causticColor = mix(vec3(0.09, 0.15, 0.22), vec3(0.24, 0.40, 0.46), dayFactor);
        col += causticColor * caustic * exp(-tHit * 0.055 * density) * 0.5;

        if (OceanWireframe > 0.5) {
            float wire = oceanWireMask(hitPos.xz, tHit);
            vec3 wireColor = vec3(0.52, 0.84, 0.96);
            col = mix(col, wireColor, wire * 0.45);
        }
    } else {
        col = mix(col, vec3(0.02, 0.09, 0.14), saturate(travel / 220.0));
    }

    float depthFade = saturate(travel / 220.0);
    col = mix(col, vec3(0.02, 0.10, 0.16), depthFade * 0.35);

    return clamp(max(col, vec3(0.01, 0.04, 0.06)), 0.0, 1.0);
}

bool sphereIntersect(vec3 ro, vec3 rd, vec3 c, float r, out float t0, out float t1) {
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float c2 = dot(oc, oc) - r * r;
    float h = b * b - c2;
    if (h < 0.0) return false;
    h = sqrt(h);
    t0 = -b - h;
    t1 = -b + h;
    return true;
}

bool cloudShellInterval(vec3 ro, vec3 rd, out float tEnter, out float tExit) {
    float rInner = EARTH_RADIUS + CloudBottom;
    float rOuter = EARTH_RADIUS + CloudTop;

    float o0, o1;
    if (!sphereIntersect(ro, rd, EarthCenter, rOuter, o0, o1)) {
        return false;
    }

    tEnter = max(o0, 0.0);
    tExit = o1;
    if (tExit <= tEnter) {
        return false;
    }

    float i0, i1;
    if (sphereIntersect(ro, rd, EarthCenter, rInner, i0, i1)) {
        bool insideInner = length(ro - EarthCenter) < rInner;
        if (insideInner) {
            tEnter = max(tEnter, i1);
        } else if (i0 > tEnter) {
            tExit = min(tExit, i0);
        } else if (i1 > tEnter && i1 < tExit) {
            tEnter = i1;
        }
    }

    return tExit > tEnter;
}

float heightFraction(vec3 worldPos) {
    float h = length(worldPos - EarthCenter) - EARTH_RADIUS;
    return saturate((h - CloudBottom) / max(CloudTop - CloudBottom, 1.0));
}

float phaseHG(float g, float cosT) {
    float g2 = g * g;
    float denom = pow(1.0 + g2 - 2.0 * g * cosT, 1.5);
    return (1.0 - g2) / max(4.0 * PI * denom, 1e-6);
}

float sampleCloudDensity(vec3 worldPos) {
    float hf = heightFraction(worldPos);
    if (hf <= 0.0 || hf >= 1.0) {
        return 0.0;
    }

    float softness = saturate(CloudSoftness);
    float storminess = saturate(CloudStorminess);
    float userDensity = max(0.01, CloudDensity);
    float userCoverage = clamp(CloudCoverage, 0.0, 2.0);

    vec3 rel = worldPos - EarthCenter;
    vec3 p = rel / 8000.0;

    vec2 curl = texture(CurlNoiseTexture, fract(p.xz * 0.05 + vec2(Time * 0.01, -Time * 0.013))).rg * 2.0 - 1.0;
    p.xz += curl * mix(0.18, 0.42, softness);

    vec4 lf = texture(lowFrequencyTexture, fract(p * 0.25 + vec3(Time * 0.01, 0.0, 0.0)));
    float base = lf.r;
    float worleyFBM = saturate(lf.g * 0.625 + lf.b * 0.25 + lf.a * 0.125);

    float threshold = mix(0.60, 0.47, storminess);
    float softnessBand = mix(0.08, 0.18, softness);
    float shape = smoothstep(threshold - softnessBand - 0.25 * worleyFBM, threshold + 0.22, base);

    vec2 wuv = fract(rel.xz / 200000.0 + 0.5);
    float coverage = texture(WeatherTexture, wuv).r;
    coverage = mix(0.16, 0.88, coverage);
    coverage = mix(coverage * 0.85, min(1.0, coverage + 0.14), storminess);
    coverage = saturate(coverage * userCoverage + (userCoverage - 1.0) * 0.28);

    float bottom = mix(0.18, 0.28, softness);
    float top = mix(0.62, 0.90, softness);
    float heightMask = smoothstep(0.0, bottom, hf) * (1.0 - smoothstep(top, 1.0, hf));
    shape *= heightMask;

    shape = saturate((shape - (1.0 - coverage)) / max(coverage, 1e-4));

    float hfNoise = texture(highFrequencyTexture, fract(p * 0.9 + vec3(0.0, Time * 0.02, 0.0))).r;
    float erosion = mix(0.30, 0.14, softness);
    shape -= (1.0 - hfNoise) * erosion;

    shape = max(0.0, shape - mix(0.030, 0.010, softness));
    shape = clamp(shape * userDensity, 0.0, 1.5);

    if (storminess > 0.01) {
        shape = mix(shape, saturate(pow(shape, 0.75) * 1.2), storminess * 0.8);
    }

    return saturate(shape);
}

void marchClouds(vec3 ro, vec3 rd, vec3 sunDir, vec3 moonDir, out vec3 cloudColor, out float cloudAlpha) {
    cloudColor = vec3(0.0);
    cloudAlpha = 0.0;

    vec3 upDir = normalize(ro - EarthCenter);
    if (dot(rd, upDir) < -0.08) {
        return;
    }

    float t0, t1;
    if (!cloudShellInterval(ro, rd, t0, t1)) {
        return;
    }

    if (rd.y < -1e-5) {
        float tOcean = -ro.y / rd.y;
        if (tOcean > 0.0) {
            t1 = min(t1, tOcean);
        }
    }

    const float maxTraceDistance = 52000.0;
    t1 = min(t1, t0 + maxTraceDistance);

    float segLen = t1 - t0;
    if (segLen <= 1.0) {
        return;
    }

    float segNorm = saturate(segLen / maxTraceDistance);
    int steps = int(mix(26.0, 72.0, segNorm));
    float stepSize = segLen / float(steps);

    float jitter = hash13(vec3(gl_FragCoord.xy, Time)) - 0.5;
    t0 += jitter * stepSize;

    float storminess = saturate(CloudStorminess);
    float softness = saturate(CloudSoftness);
    float dayFactor = dayFactorFromSun(sunDir);

    vec3 lightDir = normalize(mix(moonDir, sunDir, dayFactor));
    vec3 mainLightCol = mix(vec3(0.34, 0.40, 0.56), vec3(1.0, 0.96, 0.88), dayFactor);

    float trans = 1.0;
    vec3 accum = vec3(0.0);

    float extinction = mix(0.0010, 0.0016, storminess) * max(CloudDensity, 0.15);
    float scattering = mix(0.0008, 0.0012, softness);

    for (int i = 0; i < steps; ++i) {
        float tStep = t0 + (float(i) + 0.5) * stepSize;
        vec3 p = ro + rd * tStep;
        float dens = sampleCloudDensity(p);
        if (dens <= 0.0004) {
            continue;
        }

        float shadow = 0.0;
        vec3 lp = p;
        float lightStep = mix(700.0, 460.0, storminess);
        for (int k = 0; k < 4; ++k) {
            lp += lightDir * lightStep;
            shadow += sampleCloudDensity(lp);
        }

        float lightTrans = exp(-shadow * mix(1.10, 1.65, storminess));
        float cosT = dot(rd, lightDir);
        float phase = phaseHG(mix(0.46, 0.68, softness), cosT);
        phase = mix(phase, 1.0 / (4.0 * PI), 0.12);

        vec3 ambient = mix(vec3(0.10, 0.13, 0.18), vec3(0.60, 0.66, 0.74), dayFactor) * 0.045;
        vec3 src = (mainLightCol * lightTrans * phase + ambient) * dens;

        accum += trans * src * stepSize * scattering;
        trans *= exp(-dens * stepSize * extinction);

        if (trans < 0.01) {
            break;
        }
    }

    cloudAlpha = saturate(1.0 - trans);
    cloudColor = clamp(accum, 0.0, 6.0);
}

void main() {
    vec2 resolution = max(vec2(screenWidth, screenHeight), vec2(1.0));
    vec2 ndc = vUv * 2.0 - 1.0;
    ndc.x *= resolution.x / resolution.y;

    float waveStrength = max(0.1, OceanWaveStrength);
    float waterDensity = max(0.1, UnderwaterDensity);
    float waveTime = Time * 0.85;

    float skyHours = mod(SkyTimeHours, 24.0);
    if (skyHours < 0.0) {
        skyHours += 24.0;
    }
    vec3 sunDir = computeSunDirection(skyHours);
    vec3 moonDir = computeMoonDirection(skyHours);

    vec3 ro = cameraPosition;

    float camSurfaceH;
    vec2 camGrad;
    float camCrest;
    waveSpectrum(ro.xz, waveTime, waveStrength, camSurfaceH, camGrad, camCrest);
    float immersion = smoothstep(-0.35, 0.35, camSurfaceH - ro.y);

    vec2 underwaterDistort = vec2(0.0);
    if (immersion > 0.001) {
        underwaterDistort.x = (fbm(vUv * 6.0 + vec2(Time * 0.45, -Time * 0.20)) - 0.5) * 0.035 * immersion;
        underwaterDistort.y = (fbm(vUv * 7.0 + vec2(-Time * 0.35, Time * 0.30)) - 0.5) * 0.030 * immersion;
    }

    vec3 rd = normalize(cameraFront * 1.6 + cameraRight * (ndc.x + underwaterDistort.x) + cameraUp * (ndc.y + underwaterDistort.y));

    vec3 aboveBackground = (rd.y < 0.0) ? renderOceanAbove(ro, rd, waveTime, waveStrength, waterDensity, sunDir, moonDir) : skyColor(rd, sunDir, moonDir);
    vec3 underwaterBackground = renderUnderwater(ro, rd, waveTime, waveStrength, waterDensity, sunDir, moonDir);
    vec3 background = mix(aboveBackground, underwaterBackground, immersion);

    vec3 cloudColor = vec3(0.0);
    float cloudAlpha = 0.0;
    if (immersion < 0.98) {
        marchClouds(ro, rd, sunDir, moonDir, cloudColor, cloudAlpha);
        cloudAlpha *= (1.0 - immersion);
    }

    vec3 finalColor = background * (1.0 - cloudAlpha) + cloudColor;
    finalColor = tonemap(finalColor);
    finalColor = pow(clamp(finalColor, 0.0, 1.0), vec3(0.92));
    finalColor = max(finalColor, vec3(0.004, 0.02, 0.03));

    FragColor = vec4(finalColor, 1.0);
}
