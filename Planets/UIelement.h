#pragma once

class UIelement {
public:
	int x, y;
	int width, height;

	UIelement();
	UIelement(int X, int Y);
	UIelement(int X, int Y, int Width, int Height);
};