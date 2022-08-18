
#include "Scene.h"

Scene::~Scene()
{
	Shutdown();
}

void Scene::Shutdown()
{
	for (auto& actor : mActors)
		delete actor;

	mActors.clear();
}

void Scene::Start()
{
	for (auto& actor : mActors)
		actor->Start();
}

void Scene::Tick(const float dt)
{
	for (auto& actor : mActors)
		actor->Tick(dt);
}

void Scene::Destroy()
{
	for (auto& actor : mActors)
		actor->Destroy();
}

Actor* Scene::NewActor(const std::string& name)
{
	Actor* actor = new Actor(name);

	mActors.push_back(actor);

	return actor;
}

void Scene::Render(RenderManager* renderManager)
{
	std::vector<Actor*> renderables = View<MeshRenderer>();

	for (auto& renderable : renderables)
	{
		glm::mat4 matrix = renderable->GetTransform().ComputeMatrix(glm::mat4(1.0f));

		renderable->GetComponent<MeshRenderer>()->model->Queue(renderManager, matrix);
	}
}

Actor* Scene::FindActor(const std::string& name)
{
	for (auto& actor : mActors)
		if (actor->GetName() == name)
			return actor;

	return nullptr;
}