#pragma once



#include "Resource.h"
#include "../Renderer/Mesh.h"
#include <vector>


class Model : public Resource
{
public:

	void Discard() override;
	
	struct Submesh
	{

	};

	// All vertices are put into a single mesh and then indexed into
	Mesh* mesh;


private:
};