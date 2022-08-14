#pragma once
#include <glm/glm.hpp>

inline float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

inline glm::vec3 lerp(glm::vec3 a, glm::vec3 b, glm::vec3 t)
{
	return a + (b - a) * t;
}

template<typename _Ty>
class Rect
{
	Rect() { }
	Rect(_Ty x, _Ty y, _Ty w, _Ty h) : x(x), y(y), w(w), h(h) { }

	_Ty x, y, w, h;
};

typedef Rect<int> IntRect;