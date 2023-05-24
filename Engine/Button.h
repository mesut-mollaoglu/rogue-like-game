#pragma once
#include "Graphics.h"
#include <chrono>
#include <thread>

template <typename T>
class Button {
public:
	Button(float xPosition, float yPosition, float width, float height, ID2D1Bitmap* bitmap, std::wstring staticText = L"") :
		buttonImage(bitmap), text(staticText),
		x(xPosition), y(yPosition), nWidth(width), nHeight(height) {
		rect = D2D1::RectF(x, y, x + nWidth, y + nHeight);
		state = 1;
		assert(!std::is_same_v<decltype(this->state), std::string>);
	}
	bool Hover() {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(Structures::Window::windowHandle, &pt);
		RECT rc;
		SetRect(&rc, rect.left, rect.top, rect.right, rect.bottom);
		return PtInRect(&rc, pt);
	}
	std::wstring ChangeState() {
		auto start = std::chrono::high_resolution_clock::now();
		while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < 0.25) {
			// do nothing
		}
		if (this->Clicked()) {
			if (std::is_same_v<decltype(this->state), bool>) this->state = !this->state;
			else if (std::is_same_v<decltype(this->state), float>) this->state+=10;
		}
		return std::to_wstring(this->GetValue() % 100);
	}
	bool Clicked() {
		return (this->Hover() && GetAsyncKeyState(VK_LBUTTON));
	}
	void RenderButton() {
		if (this->buttonImage != nullptr) {
			Graphics::GetRenderTarget()->DrawBitmap(this->buttonImage, D2D1::RectF(this->x, this->y,
				nWidth + this->x, nHeight + this->y),
				this->Hover() ? 0.3f : 1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				D2D1::RectF(0, 0, nWidth, nHeight));
		}
		else {
			Graphics::GetRenderTarget()->FillRectangle(this->rect, Graphics::blackColor.Get());
			Graphics::DrawTextF(text + this->ChangeState(), rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top, Graphics::whiteColor.Get());
		}
	}
	inline T GetValue() {
		return this->state;
	}
private:
	std::wstring text;
	T state;
	ID2D1Bitmap* buttonImage;
	D2D1_RECT_F rect;
	float x, y, nWidth, nHeight;
};