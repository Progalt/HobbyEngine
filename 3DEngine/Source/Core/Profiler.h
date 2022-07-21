#pragma once

#include "Timer.h"

#include <string>
#include <unordered_map>

class Profiler
{
public:

	void BeginProfiling(const std::string& key)
	{
		mTimers[key].Start();
	}

	double EndProfiling(const std::string& key)
	{
		mTimers[key].Stop();

		return mTimers[key].ElapsedMilliseconds();
	}

	double GetMilliseconds(const std::string& key)
	{
		return mTimers[key].ElapsedMilliseconds();
	}

	double GetMicroseconds(const std::string& key)
	{
		return mTimers[key].ElapsedMicroseconds();
	}

private:

	std::unordered_map<std::string, Timer> mTimers;
};

extern Profiler gProfiler;