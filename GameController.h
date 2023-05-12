#pragma once
#include "Enemy.h"
#include "Character.h"
#include <chrono>
#include <string>
#include "MainMenu.h"

using namespace DirectX;

class GameController {
public:
	enum State {
		mainMenu,
		gameRunning
	};
	State states;
	std::unique_ptr<MainMenu> Menu;
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::unique_ptr<Character> character;
	using Clock = std::chrono::steady_clock;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> now;
	std::chrono::milliseconds duration;
	bool bInit = false;
	void Load();
	void Unload();
	void Update(HWND windowHandle, MSG msg);
	void Render();
};