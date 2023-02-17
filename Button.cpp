#include "Button.h"

Button::Button(Graphics* graphics, HWND windowHandle, float buttonX, float buttonY,	
	float width, float height, ID2D1Bitmap* bitmap, std::wstring staticText): 
	buttonImage(bitmap), hwnd(windowHandle), gfx(graphics), text(staticText),
	x(buttonX), y(buttonY), nWidth(width), nHeight(height) {
	rect = D2D1::RectF(x, y, x + nWidth, y + nHeight);
	state = false;
}

bool Button::Hover() {
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);
	RECT rc;
	SetRect(&rc, rect.left, rect.top, rect.right, rect.bottom);
	return PtInRect(&rc, pt);
}

bool Button::Clicked() {
	return (this->Hover() && GetAsyncKeyState(VK_LBUTTON));
}

std::wstring Button::ChangeState() {
	auto start = std::chrono::high_resolution_clock::now();
	while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < 0.25) {
		// do nothing
	}
	if(this->Clicked()) state = !state;
	return state ? L"ON" : L"OFF";
}

void Button::RenderButton() {
	if (this->buttonImage != nullptr){
		this->gfx->GetRenderTarget()->DrawBitmap(this->buttonImage, D2D1::RectF(this->x, this->y,
			nWidth + this->x, nHeight + this->y),
			this->Hover() ? 0.3f : 1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
			D2D1::RectF(0, 0, nWidth, nHeight));
	}else {
		this->gfx->GetRenderTarget()->FillRectangle(this->rect, this->gfx->blackColor.Get());
		this->gfx->DrawTextF(text + this->ChangeState(), rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top, this->gfx->whiteColor.Get());
	}
}