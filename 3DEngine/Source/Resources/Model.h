#pragma once



#include "Resource.h"
#include "../Renderer/Mesh.h"
#include <vector>
#include "../Model/PMDL.h"


class Model : public Resource
{
public:

	void Discard() override;
	
	void LoadFromFile(const std::string& path, RenderManager* renderManager );

	void Queue(RenderManager* renderManager, glm::mat4 matrix, uint32_t lodIndex = 0);
	
	std::vector<pmdl::Mesh> submeshes;

	std::vector<pmdl::Mesh> LODs;

	Mesh* mesh;

	std::vector<Material*> materials;

private:
};