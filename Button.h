#pragma once
#include "Graphics.h"
#include <chrono>
#include <thread>

class Button {
public:
	Button(Graphics* graphics, HWND windowHandle, float x, float y, float width, float height, ID2D1Bitmap* bitmap, std::wstring staticText);
	bool Hover();
	std::wstring ChangeState();
	bool Clicked();
	void RenderButton();
private:
	std::wstring text;
	bool state;
	ID2D1Bitmap* buttonImage;
	Graphics* gfx;
	D2D1_RECT_F rect;
	float x, y, nWidth, nHeight;
	HWND hwnd;
};