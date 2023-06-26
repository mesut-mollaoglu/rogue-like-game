#pragma once
#include"Primitives.h"

class GameController {
public:
	using Clock = std::chrono::steady_clock;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> now;
	std::chrono::milliseconds duration;
	int nLevelIndex = 0;
	std::vector<Level*> vLevels;
	bool bInit = false;
	void Load();
	void Unload();
	void Update();
	void Render();
};