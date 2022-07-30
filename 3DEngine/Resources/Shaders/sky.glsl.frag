
#version 450 core

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vSun;
layout(location = 2) in float vTime;



const float Br = 0.0020;
const float Bm = 0.0009;
const float g =  0.9200;
const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
const vec3 Km = Bm / pow(nitrogen, vec3(0.84));


void main()
{
   

	float time = vTime;

	float mu = dot(normalize(vPos), normalize(vSun));
    float rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    vec3 day_extinction = exp(-exp(-((vPos.y + vSun.y * 4.0) * (exp(-vPos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-vPos.y * 16.0) + 0.1) * Kr / Br) * exp(-vPos.y * exp(-vPos.y * 8.0 ) * 4.0) * exp(-vPos.y * 2.0) * 4.0;
    vec3 night_extinction = vec3(1.0 - exp(vSun.y)) * 0.2;
    vec3 extinction = mix(day_extinction, night_extinction, -vSun.y * 0.2 + 0.5);
    outColour.rgb = rayleigh * mie * extinction;
    outColour.rgb /= 2;
    outColour.a = 1.0;

    if (vPos.y < 0)
        outColour.rgb = vec3(0.1, 0.1, 0.1);
}