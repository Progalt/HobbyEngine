

// Plently of these PBR Functions are from the brilliant
// https://google.github.io/filament/Filament.md

#define PI 3.14159265359

vec3 SchlickRoughness(float u, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - u, 0.0, 1.0), 5.0);
}   

float Schlick(float u, float f0, float f90) 
{
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

vec3 Schlick(float u, vec3 f0) 
{
    return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

// A is roughness
float GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

float SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float Diffuse_Lambert() 
{
    return 1.0 / PI;
}

float Diffuse_Burley(float NoV, float NoL, float LoH, float roughness) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = Schlick(NoL, 1.0, f90);
    float viewScatter = Schlick(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}