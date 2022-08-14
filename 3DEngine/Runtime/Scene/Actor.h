#pragma once

#include <string>

#include "Transform.h"
#include "Component.h"
#include <vector>

class Actor
{
public:

	Actor(const std::string& name);



	const std::string& GetName() { return mName; }

	Transform& GetTransform() { return mTransform; }

	virtual void Start() {}

	virtual void Tick(const float dt) {}

	virtual void Destroy() {}


	// Component System

	// Type added must derive from Component
	template<typename _Ty>
	_Ty* AddComponent()
	{
		mComponents.push_back(new _Ty());
		mComponents[mComponents.size() - 1]->SetParentActor(this);

		return dynamic_cast<_Ty*>(mComponents[mComponents.size() - 1]);
	}

	template<typename _Ty>
	_Ty* GetComponent()
	{
		for (size_t i = 0; i < mComponents.size(); i++)
		{
			_Ty* comp = dynamic_cast<_Ty*>(mComponents[i]);
			if (comp != nullptr)
				return comp;
		}

		return nullptr;
	}

	template<typename _Ty>
	bool HasComponent()
	{
		return (GetComponent<_Ty>() != nullptr);
	}

private:

	std::string mName = "";

	Transform mTransform;

	std::vector<Component*> mComponents;
};