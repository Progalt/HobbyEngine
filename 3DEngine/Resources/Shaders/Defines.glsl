


// This was useful forum I modified for my own use
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
// https://discourse.panda3d.org/t/glsl-octahedral-normal-packing/15233

vec2 sign_not_zero(vec2 v) 
{
        return fma(step(vec2(0.0), v), vec2(2.0), vec2(-1.0));
}

vec2 encode (vec3 v)
{
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx))  * sign_not_zero(p)) : p;
}

vec3 decode(vec2 enc)
{
    vec3 v = vec3(enc.xy, 1.0 - abs(enc.x) - abs(enc.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

vec3 WorldPosFromDepth(float depth, mat4 invProj, mat4 invView, vec2 TexCoord) {
    float z = depth;// * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invProj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = invView * viewSpacePosition;

    return worldSpacePosition.xyz;
}