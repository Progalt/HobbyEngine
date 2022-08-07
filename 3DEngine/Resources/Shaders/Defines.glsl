



// This was useful forum I modified for my own use
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
// https://discourse.panda3d.org/t/glsl-octahedral-normal-packing/15233

#define CASCADE_COUNT 3

#define THREAD_GROUP_SIZE 32



vec2 sign_not_zero(vec2 v) 
{
        return fma(step(vec2(0.0), v), vec2(2.0), vec2(-1.0));
}

vec2 encode (vec3 v)
{
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
    vec2 enc =  (v.z <= 0.0) ? ((1.0 - abs(p.yx))  * sign_not_zero(p)) : p;
    enc = enc * 0.5 + 0.5;
	return enc;
}

vec3 decode(vec2 enc)
{
    enc = enc * 2 - 1;
    vec3 v = vec3(enc.xy, 1.0 - abs(enc.x) - abs(enc.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
    vec3 n = normalize(v);
	return n;
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

// Tonemapping

vec3 filmic(vec3 x) {
  vec3 X = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, vec3(2.2));
}

vec3 unreal(vec3 x) {
  return x / (x + 0.155) * 1.019;
}


vec3 uncharted2Tonemap(vec3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  float W = 11.2;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2(vec3 color) {
  const float W = 11.2;
  float exposureBias = 2.0;
  vec3 curr = uncharted2Tonemap(exposureBias * color);
  vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
  return curr * whiteScale;
}

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 cubeCoordToWorld(ivec3 cubeCoord, vec2 cubemapSize)
  {
      vec2 texCoord = vec2(cubeCoord.xy) / cubemapSize;
      texCoord = texCoord  * 2.0 - 1.0; // -1..1
      switch(cubeCoord.z)
      {
          case 0: return vec3(1.0, -texCoord.yx); // posx
          case 1: return vec3(-1.0, -texCoord.y, texCoord.x); //negx
          case 2: return vec3(texCoord.x, 1.0, texCoord.y); // posy
          case 3: return vec3(texCoord.x, -1.0, -texCoord.y); //negy
          case 4: return vec3(texCoord.x, -texCoord.y, 1.0); // posz
          case 5: return vec3(-texCoord.xy, -1.0); // negz
      }
      return vec3(0.0);
  }

  float max3(vec3 v)
  {
    return max(v.x, max(v.y, v.z));
  }

   ivec3 texCoordToCube(vec3 texCoord, vec2 cubemapSize)
  {
      vec3 abst = abs(texCoord);
      texCoord /= max3(abst);

      float cubeFace;
      vec2 uvCoord;
      if (abst.x > abst.y && abst.x > abst.z) 
      {
          // x major
          float negx = step(texCoord.x, 0.0);
          uvCoord = mix(-texCoord.zy, vec2(texCoord.z, -texCoord.y), negx);
          cubeFace = negx;
      } 
      else if (abst.y > abst.z) 
      {
          // y major
          float negy = step(texCoord.y, 0.0);
          uvCoord = mix(texCoord.xz, vec2(texCoord.x, -texCoord.z), negy);
          cubeFace = 2.0 + negy;
      } 
      else 
      {
          // z major
          float negz = step(texCoord.z, 0.0);
          uvCoord = mix(vec2(texCoord.x, -texCoord.y), -texCoord.xy, negz);
          cubeFace = 4.0 + negz;
      }
      uvCoord = (uvCoord + 1.0) * 0.5; // 0..1
      uvCoord = uvCoord * cubemapSize;
      uvCoord = clamp(uvCoord, vec2(0.0), cubemapSize - vec2(1.0));

      return ivec3(ivec2(uvCoord), int(cubeFace));
  }