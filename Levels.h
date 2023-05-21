#pragma once

#include "Button.h"
#include "Enemy.h"
#include "Character.h"

class MainMenu : public Level {
public:
	MainMenu() {
		this->nLevel = Level::ManageLevel::CurrentLevel;
	}
	float horizontalPos(ID2D1Bitmap* bitmap) {
		return (Structures::Window::GetWidth() / 2 - 7 * bitmap->GetSize().width / 2);
	}
	void UnLoad() override {
		background->Release();
		exitButton->Release();
		playButton->Release();
		settingsButton->Release();
		creditsButton->Release();
		credits->Release();
	}
	void Load() override {
		background = Sprite::LoadSprite(L"MenuContent\\main menu.png", 160, 90, this->background.Get());
		exitButton = Sprite::LoadSprite(L"MenuContent\\exit.png", 26, 13, this->exitButton.Get());
		playButton = Sprite::LoadSprite(L"MenuContent\\start.png", 34, 13, this->playButton.Get());
		settingsButton = Sprite::LoadSprite(L"MenuContent\\settings.png", 46, 13, this->settingsButton.Get());
		creditsButton = Sprite::LoadSprite(L"MenuContent\\credits.png", 40, 13, this->creditsButton.Get());
		credits = Sprite::LoadSprite(L"MenuContent\\credits_screen.png", 160, 90, this->credits.Get());
		menuStates = main;
		Vsync = new Button<bool>(Structures::Window::GetWidth() / 2 - 250, 200, 500, 20, nullptr, L"VSYNC: ");
		Play = new Button<bool>(horizontalPos(playButton.Get()), 320, 238, 91, playButton.Get());
		Credits = new Button<bool>(horizontalPos(creditsButton.Get()), 420, 280, 91, creditsButton.Get());
		Settings = new Button<bool>(horizontalPos(settingsButton.Get()), 520, 322, 91, settingsButton.Get());
		Exit = new Button<bool>(horizontalPos(exitButton.Get()), 620, 182, 91, exitButton.Get());
	}
	void Update() override {
		switch (menuStates) {
		case main: {
			if (Play->Clicked()) this->nLevel = Level::ManageLevel::NextLevel;
			else if (Credits->Clicked()) menuStates = creditsScreen;
			else if (Exit->Clicked()) PostQuitMessage(0);
			else if (Settings->Clicked()) menuStates = settings;
			break;
		}
		case creditsScreen: {
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
	void FixedUpdate() override {

	}
	void Render() override {
		Graphics::GetRenderTarget()->BeginDraw();
		switch (menuStates) {
		case main: {
			Graphics::GetRenderTarget()->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			Graphics::GetRenderTarget()->DrawBitmap(background.Get(), D2D1::RectF(0, 0,
				Structures::Window::GetWidth(), Structures::Window::GetHeight()),
				1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				D2D1::RectF(0, 0, Structures::Window::GetWidth(), Structures::Window::GetHeight()));
			Play->RenderButton();
			Credits->RenderButton();
			Exit->RenderButton();
			Settings->RenderButton();
			break;
		}
		case creditsScreen: {
			Graphics::GetRenderTarget()->DrawBitmap(credits.Get(), D2D1::RectF(0, 0,
				Structures::Window::GetWidth(), Structures::Window::GetHeight()),
				1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				D2D1::RectF(0, 0, Structures::Window::GetWidth(), Structures::Window::GetHeight()));
			break;
		}
		case settings: {
			Graphics::GetRenderTarget()->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			Graphics::DrawTextF(L"OPTIONS", Structures::Window::GetWidth() / 2 - 250,
				100, 500, 20, Graphics::whiteColor.Get());
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
		Graphics::GetRenderTarget()->EndDraw();
	}
private:
	enum MenuStates {
		main,
		exit,
		settings,
		creditsScreen,
		play
	};
	Button<bool>* Vsync;
	Button<bool>* Exit;
	Button<bool>* Play;
	Button<bool>* Settings;
	Button<bool>* Credits;
	MenuStates menuStates;
	ComPtr<ID2D1Bitmap> background;
	ComPtr<ID2D1Bitmap> exitButton;
	ComPtr<ID2D1Bitmap> playButton;
	ComPtr<ID2D1Bitmap> settingsButton;
	ComPtr<ID2D1Bitmap> creditsButton;
	ComPtr<ID2D1Bitmap> credits;
};

class GameLoop : public Level {
public:
	GameLoop() {
		this->nLevel = Level::ManageLevel::CurrentLevel;
		this->ticks = 150;
	}
	void Load() override {
		mRect = PrimitiveShapes::TexturedRect(-0.9);
		enemies.push_back(std::make_unique<Enemy>("EnemyAttackFrames", "EnemyMovingFrames", "EnemyIdleFrames", "EnemyDeadFrames", 152, 148));
		character = std::make_unique<Character>("IdleAnimFrames", "WalkingAnimFrames", "HitAnimFrames", "DashAnimFrames", 192, 138);
		nMap = Sprite::LoadTexture(L"map.png", 3360, 1890);
	}
	void UnLoad() override {

	}
	void FixedUpdate() override {
		for (int i = 0; i < enemies.size(); i++) {
			enemies[i]->Update(character->GetPosition());
			if (Sprite::CheckCollision(character.get(), enemies[i].get())) {
				if (character.get()->GetStateMachine().equals("Dash")) {
					enemies[i]->health = 0;
				}
				if (character.get()->GetStateMachine().equals("Attack")) {
					enemies[i]->health -= 10;
				}
			}
		}
		character->Update();
	}
	void Render() override {
		using Structures::Camera;
		Graphics::SetConstantValues<Graphics::ProjectionBuffer>(Graphics::projectionBuffer.Get(), {
			Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		Graphics::Clear(0.5f, 0.5f, 0.5f, 1.0f);
		Graphics::Begin();
		mRect.Draw(nMap);
		character->Render();
		Graphics::d3dDeviceContext->PSSetShader(Graphics::pixelShader.Get(), nullptr, 0);
		for (int i = 0; i < enemies.size(); i++) {
			enemies[i].get()->Render();
		}
		Graphics::End();
	}
	void Update() override {

	}
private:
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::unique_ptr<Character> character;
	ID3D11ShaderResourceView* nMap;
	PrimitiveShapes::TexturedRect mRect;
};