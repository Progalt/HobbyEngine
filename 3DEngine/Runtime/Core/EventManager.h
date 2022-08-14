#pragma once

#include "KeyCodes.h"

#include "../Core/Singleton.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_set>

	enum class EventType
	{
		KeyboardInput,
		MouseMove,
		MouseButton,

		WindowResize,

		Count
	};

	enum class ButtonState
	{
		Pressed, Released
	};

	class EventListener;

	class Event
	{
	public:

		explicit Event(EventType type) : type(type) { }

		EventType type;
	};

#define DECLARE_EVENT using Event::Event

	class KeyboardEvent : public Event
	{
	public:

		DECLARE_EVENT;

		KeyCode keycode;
		uint32_t modifier;
		ButtonState buttonState;

	};

	class MouseMoveEvent : public Event
	{
	public:

		DECLARE_EVENT;

		void SetZero()
		{
			x = 0, y = 0;
			dx = 0, y = 0;
		}

		// Current coordinate of the mouse relative to the window client area
		uint32_t x, y;

		// Delta mouse movement based on last input event
		uint32_t dx, dy;
	};

	class MouseButtonEvent : public Event
	{
	public:

		DECLARE_EVENT;

		MouseButton button;
		ButtonState state;
	};

	class WindowResizeEvent : public Event
	{
	public:

		DECLARE_EVENT;

		uint32_t newWidth, newHeight;
	};


	class EventManager
	{
	public:

		DECLARE_SINGLETON(EventManager)


		void Init();

		void SendEvent(std::shared_ptr<Event> ev);

		void Attach(EventListener* listener, EventType evtype);


	private:

		friend EventListener;

		std::vector<std::unordered_set<EventListener*>> mEventListeners;

	};

	class EventListener
	{
	public:

		virtual void OnEvent(std::shared_ptr<Event> ev) = 0;


	};