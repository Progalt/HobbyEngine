#pragma once

#include "EventManager.h"
#include <glm/glm.hpp>

class Input : public EventListener
{
public:

	void OnEvent(std::shared_ptr<Event> ev) override;

	bool IsKeyPressed(KeyCode code);

	bool IsButtonPressed(MouseButton button);

	glm::ivec2 GetMousePosition();

private:

	bool keys[(int)KeyCode::__Count];

	bool mouseButtons[4];

};

extern Input input;