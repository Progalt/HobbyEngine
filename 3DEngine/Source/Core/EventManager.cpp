

#include "EventManager.h"


	void EventManager::Init()
	{
		mEventListeners.resize((size_t)EventType::Count);


	}

	void EventManager::SendEvent(std::shared_ptr<Event> ev)
	{
		for (auto& listener : mEventListeners[(size_t)ev->type])
		{
			listener->OnEvent(ev);
		}
	}

	void EventManager::Attach(EventListener* listener, EventType evtype)
	{
		if (mEventListeners.size() == 0)
			Init();

		mEventListeners[(size_t)evtype].emplace(listener);
	}

