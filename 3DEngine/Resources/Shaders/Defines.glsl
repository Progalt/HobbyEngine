




#define CASCADE_COUNT 3
#define DEPTH_MAX 1.0

#define THREAD_GROUP_SIZE 32

vec2 poisson_disk[] = vec2[64]
(
    vec2(-0.5119625f, -0.4827938f),
    vec2(-0.2171264f, -0.4768726f),
    vec2(-0.7552931f, -0.2426507f),
    vec2(-0.7136765f, -0.4496614f),
    vec2(-0.5938849f, -0.6895654f),
    vec2(-0.3148003f, -0.7047654f),
    vec2(-0.42215f, -0.2024607f),
    vec2(-0.9466816f, -0.2014508f),
    vec2(-0.8409063f, -0.03465778f),
    vec2(-0.6517572f, -0.07476326f),
    vec2(-0.1041822f, -0.02521214f),
    vec2(-0.3042712f, -0.02195431f),
    vec2(-0.5082307f, 0.1079806f),
    vec2(-0.08429877f, -0.2316298f),
    vec2(-0.9879128f, 0.1113683f),
    vec2(-0.3859636f, 0.3363545f),
    vec2(-0.1925334f, 0.1787288f),
    vec2(0.003256182f, 0.138135f),
    vec2(-0.8706837f, 0.3010679f),
    vec2(-0.6982038f, 0.1904326f),
    vec2(0.1975043f, 0.2221317f),
    vec2(0.1507788f, 0.4204168f),
    vec2(0.3514056f, 0.09865579f),
    vec2(0.1558783f, -0.08460935f),
    vec2(-0.0684978f, 0.4461993f),
    vec2(0.3780522f, 0.3478679f),
    vec2(0.3956799f, -0.1469177f),
    vec2(0.5838975f, 0.1054943f),
    vec2(0.6155105f, 0.3245716f),
    vec2(0.3928624f, -0.4417621f),
    vec2(0.1749884f, -0.4202175f),
    vec2(0.6813727f, -0.2424808f),
    vec2(-0.6707711f, 0.4912741f),
    vec2(0.0005130528f, -0.8058334f),
    vec2(0.02703013f, -0.6010728f),
    vec2(-0.1658188f, -0.9695674f),
    vec2(0.4060591f, -0.7100726f),
    vec2(0.7713396f, -0.4713659f),
    vec2(0.573212f, -0.51544f),
    vec2(-0.3448896f, -0.9046497f),
    vec2(0.1268544f, -0.9874692f),
    vec2(0.7418533f, -0.6667366f),
    vec2(0.3492522f, 0.5924662f),
    vec2(0.5679897f, 0.5343465f),
    vec2(0.5663417f, 0.7708698f),
    vec2(0.7375497f, 0.6691415f),
    vec2(0.2271994f, -0.6163502f),
    vec2(0.2312844f, 0.8725659f),
    vec2(0.4216993f, 0.9002838f),
    vec2(0.4262091f, -0.9013284f),
    vec2(0.2001408f, -0.808381f),
    vec2(0.149394f, 0.6650763f),
    vec2(-0.09640376f, 0.9843736f),
    vec2(0.7682328f, -0.07273844f),
    vec2(0.04146584f, 0.8313184f),
    vec2(0.9705266f, -0.1143304f),
    vec2(0.9670017f, 0.1293385f),
    vec2(0.9015037f, -0.3306949f),
    vec2(-0.5085648f, 0.7534177f),
    vec2(0.9055501f, 0.3758393f),
    vec2(0.7599946f, 0.1809109f),
    vec2(-0.2483695f, 0.7942952f),
    vec2(-0.4241052f, 0.5581087f),
    vec2(-0.1020106f, 0.6724468f)
);

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
    z = 1.0 - z;

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