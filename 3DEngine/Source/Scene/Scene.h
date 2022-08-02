#pragma once

#include "Actor.h"
#include <vector>

class Scene
{
public:

	~Scene();

	void Shutdown();

	void Start();

	void Tick(const float dt);

	void Destroy();

	Actor* NewActor(const std::string& name);

	template<typename _Ty>
	std::vector<Actor*> View()
	{
		std::vector<Actor*> output;
		for (auto& actor : mActors)
		{
			if (actor->HasComponent<_Ty>())
				output.push_back(actor);
		}

		return output;
	}

private:

	// Full list of all actors in the scene.
	// Not a scene tree
	std::vector<Actor*> mActors;

};