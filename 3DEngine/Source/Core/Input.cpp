
#include "Input.h"

Input input;

void Input::OnEvent(std::shared_ptr<Event> ev)
{
	if (ev->type == EventType::KeyboardInput)
	{
		std::shared_ptr<KeyboardEvent> keyboard = std::static_pointer_cast<KeyboardEvent>(ev);
		keys[(int)keyboard->keycode] = (keyboard->buttonState == ButtonState::Pressed) ? true : false;
	}

	if (ev->type == EventType::MouseButton)
	{
		std::shared_ptr<MouseButtonEvent> keyboard = std::static_pointer_cast<MouseButtonEvent>(ev);
		mouseButtons[(int)keyboard->button] = (keyboard->state == ButtonState::Pressed) ? true : false;
	}
}

bool Input::IsKeyPressed(KeyCode code)
{
	return keys[int(code)];
}

bool Input::IsButtonPressed(MouseButton button)
{
	return mouseButtons[(int)button];
}

glm::ivec2 Input::GetMousePosition()
{
	glm::ivec2 out = { 0, 0};

	SDL_GetMouseState(&out.x, &out.y);

	return out;
}