#pragma once

#include <atomic>

class Resource
{
public:

	virtual void Discard() = 0;

	std::atomic<bool> ready = false;

private:
};