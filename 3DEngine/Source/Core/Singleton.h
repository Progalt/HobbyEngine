#pragma once


#define DECLARE_SINGLETON(type) \
type() = default;\
type(const type&) = delete; \
void operator=(const type&) = delete; \
static type& GetInstance(){ \
	static type instance; \
	return instance; \
}