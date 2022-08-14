
struct SceneInfo
{
	uint hasDirectionalLight;
	uint lightCount;
};

struct PointLight
{
	vec3 position;
	float radius;

	vec4 colour;
};

struct DirectionalLight
{
	vec4 direction;
	vec4 colour;
};