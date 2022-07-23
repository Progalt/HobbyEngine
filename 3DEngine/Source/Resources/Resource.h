#pragma once


class Resource
{
public:

	virtual void Discard() = 0;

	bool ready = false;

private:
};