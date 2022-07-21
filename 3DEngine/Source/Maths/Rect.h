#pragma once



template<typename _Ty>
class Rect
{
	Rect() { }
	Rect(_Ty x, _Ty y, _Ty w, _Ty h) : x(x), y(y), w(w), h(h) { }

	_Ty x, y, w, h;
};

typedef Rect<int> IntRect;