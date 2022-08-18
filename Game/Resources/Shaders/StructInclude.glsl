
struct SceneInfo
{
	uint hasDirectionalLight;
	uint lightCount;
};

struct PointLight
{
	vec4 position;
	vec4 colour;
};

struct DirectionalLight
{
	vec4 direction;
	vec4 colour;
};