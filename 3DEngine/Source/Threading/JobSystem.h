#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "ThreadSafeQueue.h"
#include <atomic>

class JobSystem
{
public:

	static uint32_t GetThreadCount();

	static void Init();

	static void Execute(std::function<void()> job);

	static void Poll();

	static bool IsBusy();

	static void Wait();

};