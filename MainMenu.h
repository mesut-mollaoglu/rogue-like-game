#pragma once
#include "Sprite.h"
#include "Button.h"

class MainMenu {
public:
	MainMenu(Sprite* sprite, Graphics* graphics, HWND windowHandle):spriteLoader(sprite), gfx(graphics), hwnd(windowHandle){
		background = spriteLoader->LoadSprite(L"MenuContent\\main menu.png", 160, 90, this->background.Get());
		exitButton = spriteLoader->LoadSprite(L"MenuContent\\exit.png", 26, 13, this->exitButton.Get());
		playButton = spriteLoader->LoadSprite(L"MenuContent\\start.png", 34, 13, this->playButton.Get());
		settingsButton = spriteLoader->LoadSprite(L"MenuContent\\settings.png", 46, 13, this->settingsButton.Get());
		creditsButton = spriteLoader->LoadSprite(L"MenuContent\\credits.png", 40, 13, this->creditsButton.Get());
		credits = spriteLoader->LoadSprite(L"MenuContent\\credits_screen.png", 160, 90, this->credits.Get());
		menuStates = main;
		Vsync = new Button(gfx, hwnd, gfx->renderTargetWidth / 2 - 250, 200, 500, 20, nullptr, L"VSYNC: ");
		Play = new Button(gfx, hwnd, horizontalPos(playButton.Get()), 320, 238, 91, playButton.Get(), L"");
		Credits = new Button(gfx, hwnd, horizontalPos(creditsButton.Get()), 420, 280, 91, creditsButton.Get(), L"");
		Settings = new Button(gfx, hwnd, horizontalPos(settingsButton.Get()), 520, 322, 91, settingsButton.Get(), L"");
		Exit = new Button(gfx, hwnd, horizontalPos(exitButton.Get()), 620, 182, 91, exitButton.Get(), L"");
	}
	float horizontalPos(ID2D1Bitmap* bitmap) {
		return (gfx->renderTargetWidth / 2 - 7 * bitmap->GetSize().width / 2);
	}
	void Update() {
		switch (menuStates) {
		case main: {
			if (Play->Clicked()) bGameRunning = true;
			else if (Credits->Clicked()) menuStates = creditsScreen;
			else if (Exit->Clicked()) PostQuitMessage(0);
			else if (Settings->Clicked()) menuStates = settings;
			break;
		}
		case creditsScreen:{
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
				menuStates = main;
			}
			break;
		}
		case settings: {
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
				menuStates = main;
			}
			break;
		}
		}
	}
	void Render() {
		this->gfx->GetRenderTarget()->BeginDraw();
		switch (menuStates) {
		case main: {
			this->gfx->GetRenderTarget()->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			this->gfx->GetRenderTarget()->DrawBitmap(background.Get(), D2D1::RectF(0, 0,
				gfx->renderTargetWidth, gfx->renderTargetHeight),
				1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				D2D1::RectF(0, 0, gfx->renderTargetWidth, gfx->renderTargetHeight));
			Play->RenderButton();
			Credits->RenderButton();
			Exit->RenderButton();
			Settings->RenderButton();
			break;
		}
		case creditsScreen: {
			this->gfx->GetRenderTarget()->DrawBitmap(credits.Get(), D2D1::RectF(0, 0,
				gfx->renderTargetWidth, gfx->renderTargetHeight), 
				1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, 
				D2D1::RectF(0, 0, gfx->renderTargetWidth, gfx->renderTargetHeight));
			break;
		}
		case settings: {
			this->gfx->GetRenderTarget()->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			this->gfx->DrawTextF(L"OPTIONS", gfx->renderTargetWidth / 2 - 250, 
				100, 500, 20, gfx->whiteColor.Get());
			Vsync->RenderButton();
			break;
		}
		case exit: {
			background->Release();
			exitButton->Release();
			playButton->Release();
			settingsButton->Release();
			creditsButton->Release();
			credits->Release();
		}
		}
		this->gfx->GetRenderTarget()->EndDraw();
	}
	bool bGameRunning;
private:
	HWND hwnd;
	enum MenuStates {
		main, 
		exit,
		settings,
		creditsScreen,
		play	
	};
	Button* Exit;
	Button* Play;
	Button* Settings;
	Button* Credits;
	Button* Vsync;
	MenuStates menuStates;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> background;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> exitButton;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> playButton;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> settingsButton;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> creditsButton;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> credits;
	Graphics* gfx;
	Sprite* spriteLoader;
};