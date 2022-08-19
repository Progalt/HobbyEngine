
#include "DebugRenderer.h"

void DebugRenderer::DrawLine(const glm::vec3& p1, const glm::vec3& p2, glm::vec4 colour)
{
	vertices.push_back({ p1, colour });
	vertices.push_back({ p2, colour });
}

void DebugRenderer::DrawBox(const glm::vec3& min, const glm::vec3& max, glm::vec4 colour)
{
	// Not a complete box because im too stupid to figure out vertices

	DrawLine({ min.x, min.y, min.z }, { min.x, max.y, min.z }, colour);
	DrawLine({ max.x, min.y, min.z }, { max.x, max.y, min.z }, colour);
	DrawLine({ min.x, min.y, max.z }, { min.x, max.y, max.z }, colour);
	DrawLine({ max.x, min.y, max.z }, { max.x, max.y, max.z }, colour);

	DrawLine({ min.x, min.y, min.z }, { max.x, min.y, min.z }, colour);
	DrawLine({ min.x, min.y, max.z }, { max.x, min.y, max.z }, colour);
	DrawLine({ min.x, max.y, min.z }, { max.x, max.y, min.z }, colour);
	DrawLine({ min.x, max.y, max.z }, { max.x, max.y, max.z }, colour);

	DrawLine({ min.x, min.y, min.z }, { min.x, min.y, max.z }, colour);
	//DrawLine({ min.x, min.y, min.z }, { min.x, min.y, min.z }, colour);

	DrawLine({ min.x, max.y, min.z }, { min.x, max.y, max.z }, colour);
	//DrawLine({ min.x, max.y, min.z }, { min.x, max.y, min.z }, colour);
}