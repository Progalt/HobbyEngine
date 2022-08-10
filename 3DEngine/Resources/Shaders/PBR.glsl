

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
    return max(f0 + (1.0 - f0) * pow(2, (-5.55473 * u - 6.98316) * u), 0.0);
}

// A is roughness
float GGX(float NoH, float a) {
   float r = (a + 1.0);
	float k = (a * a) / 8.0;

	float numerator = NoH;
	float denominator = NoH * (1.0 - k) + k;

	return numerator / max(denominator, 0.001);
}

float SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}
float GeometrySchlickGGX(float cosTheta, float roughness) {
	float r = (roughness + 1.0);
	float k = (roughness * roughness) / 8.0;

	float numerator = cosTheta;
	float denominator = cosTheta * (1.0 - k) + k;

	return numerator / max(denominator, 0.001);
}
float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
	return GeometrySchlickGGX(max(dot(normal, viewDir), 0.0), roughness) * GeometrySchlickGGX(max(dot(normal, lightDir), 0.0), roughness);
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