#include "UIelement.h"

UIelement::UIelement() {
	x = 0;
	y = 0;
	width = 0;
	height = 0;
}

UIelement::UIelement(int X, int Y) {
	x = X;
	y = Y;
	width = 0;
	height = 0;
}

UIelement::UIelement(int X, int Y, int Width, int Height) {
	x = X;
	y = Y;
	width = Width;
	height = Height;
}