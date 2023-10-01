#include "Character.h"
#include "Engine/SaveFiles.h"
#include "GameSystems.h"

FontData* Data::fontData;

class Main : public Level {
public:
	Main() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		LoadFontData();
		s = SaveFile("NewFile.txt");
		pauseFile = SaveFile("PauseFile.txt");
		mRect = Sprite();
		coins = Coins();
		LoadData();
		w = WaveSystem(nMaxWaves);
		mRect.SetTexture(Graphics::LoadTexture("map.png"), 4.5f);
		chest = Chest();
		chest.mFunction = [&, this]() {if (GetTimeLapse(Clock::now(), tp) < .5f || coins.nAmount < 2) return; coins.SetAmount(coins.nAmount - 2);  tp = Clock::now(); };
		character = std::make_unique<Character>("cIdle", "cWalk", "cHit", "cDash", nHealth);
		character->SetSpeed(nSpeed);
		character->SetHealth(nHealth);
		chest.vPowerups = { Powerup("Powerups\\fastRun.png", [&, this]() {character->SetSpeed(nSpeed * 2); }, [&, this]() {character->SetSpeed(nSpeed); }, 80000) ,
		Powerup("Powerups\\shield.png", [&, this]() {bDamageEnabled = false; }, [&, this]() {bDamageEnabled = true; }, 80000) ,
		Powerup("Powerups\\health.png", [&, this]() {character->SetHealth(nHealth); }, [&, this]() {character->SetSpeed(nSpeed); }, 80000) };
		Graphics::SetStates();
		ResumeGame();
	}
	void UnLoad() override {
		FreeFontData();
		SaveData();
		s.UpdateFile();
		pauseFile.UpdateFile();
		chest.Free();
		mRect.Free();
		character->Destroy();
		w.text.Free();
		if (!w.enemies.empty())
			for (int i = 0; i < w.enemies.size(); i++) {
				w.enemies[i]->Destroy();
				w.enemies.erase(w.enemies.begin() + i);
			}
		coins.Free();
	}
	void FixedUpdate() override {
		for (std::size_t i = 0; i < w.enemies.size(); i++) {
			if (character->isState("Dash") && CheckCollision(138, 148, character->GetPosition(), w.enemies[i]->GetPosition()))
				w.enemies[i]->health = 0;
			if (character->isState("Attack") && CheckCollision(138, 148, character->GetPosition(), w.enemies[i]->GetPosition()))
				w.enemies[i]->health -= character->nDamage;
			if (bDamageEnabled && w.enemies[i]->isState("Attack") && CheckCollision(138, 148, character->GetPosition(), w.enemies[i]->GetPosition()))
				character->SetHealth(character->GetHealth() - w.enemies[i]->nDamage);
			if (w.enemies[i]->health <= 0)
				w.enemies[i]->SetState("Dead");
			if (w.enemies[i]->isState("Dead") && w.enemies[i]->AnimEnd()) {
				w.enemies[i]->Destroy();
				w.enemies.erase(w.enemies.begin() + i);
				coins.SetAmount(coins.nAmount + nCoinIncrement);
			}
		}
		if (!w.enemies.empty())
			for (auto& enemy : w.enemies)
				enemy->Update(character->GetPosition());
		character->Update();
		chest.Update(coins.nAmount >= 2, character->GetPosition());
		if (w.bFinished) {
			nLevel = Level::ManageLevel::GotoLevel;
			nIndex = 5;
		}
		w.Update(character->GetPosition());
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		mRect.Draw();
		chest.Render(character->GetPosition());
		w.Render();
		coins.Render();
		character->Render();
		if (!w.enemies.empty())
			for (auto& enemy : w.enemies)
				enemy->Render();
		character->RenderHealth();
		Graphics::End();
	}
	uint8_t CheckCollision(float cWidth, float eWidth, Vec2f cPos, Vec2f ePos) {
		float radius = (cWidth + eWidth) * 3;
		Vec2f vec = cPos - ePos;
		if ((radius * radius) > (vec.GetLengthSq())) return 1;
		return 0;
	}
	void Update() override {
		if (isKeyPressed(VK_ESCAPE))
			PauseGame();
		if (character->GetHealth() <= 0)
			nLevel = Level::ManageLevel::NextLevel;
	}
	inline void SaveData() {
		s.OverWrite("Coins: " + std::to_string(coins.nAmount) + "\n", s.GetNewLine(5));
	}
	inline void LoadData() {
		nSpeed = atoi(s.ReadBetween(s.FindEnd("Speed: "), s.GetNewLine(2)-1).c_str());
		nHealth = atoi(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(3) - 1).c_str());
		nCoinIncrement = atoi(s.ReadBetween(s.FindEnd("Increment: "), s.GetNewLine(4)-1).c_str());
		nMaxWaves = atoi(s.ReadBetween(s.FindEnd("Max Waves: "), s.GetNewLine(5)-1).c_str());
		coins.SetAmount(atoi(s.ReadBetween(s.FindEnd("Coins: "), s.GetNewLine(6)-1).c_str()));
	}
	inline void ResumeGame() {
		if (pauseFile.isEmpty()) {
			return;
		}
		int nEnemyNumber = (pauseFile.GetLineBreaks() - 7) / 4;
		int index = 2;
		if (nEnemyNumber != 0)
			for (int i = 0; i < nEnemyNumber; i++) {
				w.enemies.push_back(std::make_unique<Enemy>("eAttack", "eMove", "eIdle", "eDead"));
				w.enemies.back()->SetPosition(Vec2f::toVector(pauseFile.ReadBetween(pauseFile.FindEnd("Enemy " + std::to_string(i) + " Position: "), pauseFile.GetNewLine(index) - 1)));
				w.enemies.back()->SetState(pauseFile.ReadBetween(pauseFile.FindEnd("Enemy " + std::to_string(i) + " State: "), pauseFile.GetNewLine(index + 1) - 1).c_str());
				w.enemies.back()->SetHealth(atof(pauseFile.ReadBetween(pauseFile.FindEnd("Enemy " + std::to_string(i) + " Health: "), pauseFile.GetNewLine(index + 2) - 1).c_str()));
				w.enemies.back()->facingRight = (pauseFile.ReadBetween(pauseFile.FindEnd("Enemy " + std::to_string(i) + " Facing: "), pauseFile.GetNewLine(index + 3) - 1) == "Right" ? true : false);
				index += 4;
			}
		character->SetPosition(Vec2f::toVector(pauseFile.ReadBetween(pauseFile.FindEnd("Character Position: "), pauseFile.GetNewLine(index) - 1)));
		character->SetState(pauseFile.ReadBetween(pauseFile.FindEnd("Character State: "), pauseFile.GetNewLine(index + 1) - 1).c_str());
		character->SetHealth(atof(pauseFile.ReadBetween(pauseFile.FindEnd("Character Health: "), pauseFile.GetNewLine(index + 2) - 1).c_str()));
		character->facingRight = (pauseFile.ReadBetween(pauseFile.FindEnd("Character Facing: "), pauseFile.GetNewLine(index + 3) - 1) == "Right" ? true : false);
		int nWaveNumber = atoi(pauseFile.ReadBetween(pauseFile.FindEnd("Wave Number: "), pauseFile.GetNewLine(index + 4) - 1).c_str());
		int nSpawned = atoi(pauseFile.ReadBetween(pauseFile.FindEnd("Spawned: "), pauseFile.GetNewLine(index + 5) - 1).c_str());
		int nState = atoi(pauseFile.ReadBetween(pauseFile.FindEnd("Wave State: "), pauseFile.GetNewLine(index + 6) - 1).c_str());
		w.SetWave(nWaveNumber, static_cast<WaveSystem::WaveStates>(nState), nSpawned);
		pauseFile.Clear();
	}
	inline void PauseGame() {
		int index = 1;
		for (int i = 0; i < w.enemies.size(); i++) {
			pauseFile.OverWrite("Enemy " + std::to_string(i) + " Position: " + w.enemies[i]->GetPosition().toString() + "\n", pauseFile.GetNewLine(index));
			pauseFile.OverWrite("Enemy " + std::to_string(i) + " State: " + std::string(w.enemies[i]->GetState()) + "\n", pauseFile.GetNewLine(index + 1));
			pauseFile.OverWrite("Enemy " + std::to_string(i) + " Health: " + std::to_string(w.enemies[i]->GetHealth()) + "\n", pauseFile.GetNewLine(index + 2));
			pauseFile.OverWrite("Enemy " + std::to_string(i) + " Facing: " + std::string(w.enemies[i]->facingRight ? "Right" : "Left") + "\n", pauseFile.GetNewLine(index + 3));
			index += 4;
		}
		pauseFile.OverWrite("Character Position: " + character->GetPosition().toString() + "\n", pauseFile.GetNewLine(index));
		pauseFile.OverWrite("Character State: " + std::string(character->GetState()) + "\n", pauseFile.GetNewLine(index + 1));
		pauseFile.OverWrite("Character Health: " + std::to_string(character->GetHealth()) + "\n", pauseFile.GetNewLine(index + 2));
		pauseFile.OverWrite("Character Facing: " + std::string(character->facingRight ? "Right" : "Left") + "\n", pauseFile.GetNewLine(index + 3));
		pauseFile.OverWrite("Wave Number: " + std::to_string(w.nWaveNumber) + "\n", pauseFile.GetNewLine(index + 4));
		pauseFile.OverWrite("Spawned: " + std::to_string(w.nCurrentSpawnNumber) + "\n", pauseFile.GetNewLine(index + 5));
		pauseFile.OverWrite("Wave State: " + std::to_string((int)w.waveStates) + "\n", pauseFile.GetNewLine(index + 6));
		nLevel = Level::ManageLevel::GotoLevel;
		nIndex = 6;
	}
private:
	SaveFile s, pauseFile;
	WaveSystem w;
	std::unique_ptr<Character> character;
	Sprite mRect;
	Chest chest;
	Coins coins;
	bool bDamageEnabled = true;
	float nSpeed, nHealth;
	int nCoinIncrement, nMaxWaves;
	timePoint tp;
};

class Pause : public Level {
public:
	Pause() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		LoadFontData();
		paused = Text("Paused");
		paused.SetPosition(ToScreenCoord({ 512 - paused.GetStringSize(), 40 }));
		home = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "MenuContent\\homeIcon.png", ToScreenCoord({312, 384}));
		resume = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 1; }, "MenuContent\\resumeIcon.png", ToScreenCoord({ 712, 384 }));
	}
	void UnLoad() override {
		FreeFontData();
		paused.Free();
		home.Free();
		resume.Free();
	}
	void Update() override {
		
	}
	void FixedUpdate() override {
		resume.Update();
		home.Update();
	}
	void Render() override {
		Graphics::ClearAndBegin({0.f, 0.f, 0.f, 1.f});
		home.Draw();
		resume.Draw();
		paused.DrawString();
		Graphics::End();
	}
private:
	Text paused;
	Button home;
	Button resume;
};

class Credits : public Level {
public:
	Credits() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		Graphics::SetStates();
		sprite = Sprite();
		back = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "newMenu\\back.png", ToScreenCoord({ 920, 700 }), 0.5f);
		sprite.SetTexture(Graphics::LoadTexture("MenuContent\\credits_screen.png"), 3.0f);
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		sprite.Draw();
		back.Draw();
		Graphics::End();
	}
	void FixedUpdate() override {
		back.Update();
	}
	void Update() override {

	}
	void UnLoad() override {
		sprite.Free();
		back.Free();
	}
private:
	Sprite sprite;
	Button back;
};

class MainMenu : public Level {
public:
	MainMenu() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		start = Button([&, this]() { nLevel = Level::ManageLevel::NextLevel; }, "newMenu\\start.png", ToScreenCoord({ 512, 300 }), 0.5f);
		credits = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 3; }, "newMenu\\credits.png", ToScreenCoord({ 512, 375 }), 0.5f);
		market = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 4; }, "newMenu\\market.png", ToScreenCoord({ 512, 450 }), 0.5f);
		exit = Button([&, this]() { PostQuitMessage(0); }, "newMenu\\exit.png", ToScreenCoord({ 512, 525 }), 0.5f);
		sprite = Sprite();
		sprite.SetTexture(Graphics::LoadTexture("MenuContent\\main menu.png"), 4.0f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		sprite.Draw();
		start.Draw();
		credits.Draw();
		market.Draw();
		exit.Draw();
		Graphics::End();
	}
	void FixedUpdate() override {
		start.Update();
		credits.Update();
		market.Update();
		exit.Update();
	}
	void Update() override {

	}
	void UnLoad() override {
		start.Free();
		credits.Free();
		market.Free();
		exit.Free();
		sprite.Free();
	}
private:
	Button start;
	Button credits;
	Button exit;
	Button market;
	Sprite sprite;
};

class Marketplace : public Level {
public:
	Marketplace() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		s = SaveFile("NewFile.txt");
		LoadFontData();		
		nItemIndex = 0;
		c = Coins();
		vItems = { Item("Powerups\\fastRun.png", "Increases Walking Speed.", {10, 15, 20, 25}), 
			Item("Powerups\\health.png", "Increases Maximum Health.", {300, 400, 500, 600}),
			Item("Powerups\\moneyIcon.png", "Amount of money you earn by killing enemies.", {1, 2, 3, 4, 5}),
			Item("Powerups\\waveIcon.png", "Max number of waves.", {5, 8, 11, 14, 17}) };
		LoadData();
		back = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "newMenu\\back.png", ToScreenCoord({ 920, 700 }), 0.5f);
		left = Button([&, this]() {if (GetTimeLapse(Clock::now(), tp1) < .5f) return; if (nItemIndex > 0) nItemIndex--; tp1 = Clock::now(); }, "marketUI\\left.png", ToScreenCoord({ 64, 368 }), 1.f);
		right = Button([&, this]() {if (GetTimeLapse(Clock::now(), tp1) < .5f) return; if (nItemIndex < vItems.size() - 1) nItemIndex++; tp1 = Clock::now(); }, "marketUI\\right.png", ToScreenCoord({ 960, 368 }), 1.f);
		buy = Button([&, this]() {if (GetTimeLapse(Clock::now(), tp2) < .5f || c.nAmount < (2 << vItems[nItemIndex].nValueIndex)) return; 
		if (vItems[nItemIndex].vValues.size() > (uint64_t)vItems[nItemIndex].nValueIndex + 1) c.SetAmount(c.nAmount - (2 << vItems[nItemIndex].nValueIndex)); vItems[nItemIndex].SetLevel(vItems[nItemIndex].nValueIndex + 1);
		tp2 = Clock::now(); }, "marketUI\\buy.png", ToScreenCoord({ 512, 700 }), .5f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		back.Draw();
		vItems[nItemIndex].mIcon.Draw();
		vItems[nItemIndex].sDescription.DrawString();
		vItems[nItemIndex].sLevel.DrawString();
		if(vItems[nItemIndex].vValues.size() > (uint64_t)vItems[nItemIndex].nValueIndex+1) 
			vItems[nItemIndex].sPrice.DrawString();
		left.Draw();
		right.Draw();
		buy.Draw();
		c.Render();
		Graphics::End();
	}
	void FixedUpdate() override {
		back.Update();
		left.Update();
		right.Update();
		buy.Update();
	}
	void Update() override {
		
	}
	void UnLoad() override {
		FreeFontData();
		SaveData();
		s.UpdateFile();
		back.Free();
		for (auto& element : vItems)
			element.Free();
		left.Free();
		right.Free();
		buy.Free();
	}
	inline void LoadData() {
		vItems[0].SetLevel(vItems[0].FindValue(atof(s.ReadBetween(s.FindEnd("Speed: "), s.GetNewLine(2) - 1).c_str())));
		vItems[1].SetLevel(vItems[1].FindValue(atof(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(3) - 1).c_str())));
		vItems[2].SetLevel(vItems[2].FindValue(atoi(s.ReadBetween(s.FindEnd("Increment: "), s.GetNewLine(4)- 1).c_str())));
		vItems[3].SetLevel(vItems[3].FindValue(atoi(s.ReadBetween(s.FindEnd("Max Waves: "), s.GetNewLine(5) - 1).c_str())));
		c.SetAmount(atoi(s.ReadBetween(s.FindEnd("Coins: "), s.GetNewLine(6) - 1).c_str()));
	}
	inline void SaveData() {
		s.OverWrite("Speed: " + std::to_string(vItems[0].GetCurrentValue()) + "\n");
		s.OverWrite("Health: " + std::to_string(vItems[1].GetCurrentValue()) + "\n", s.GetNewLine(2));
		s.OverWrite("Increment: " + std::to_string((int)vItems[2].GetCurrentValue()) + "\n", s.GetNewLine(3));
		s.OverWrite("Max Waves: " + std::to_string((int)vItems[3].GetCurrentValue()) + "\n", s.GetNewLine(4));
		s.OverWrite("Coins: " + std::to_string(c.nAmount) + "\n", s.GetNewLine(5));
	}
private:
	SaveFile s;
	timePoint tp1, tp2;
	std::vector<Item> vItems;
	Button buy;
	Button left;
	Button right;
	Button back;
	Coins c;
	int nItemIndex;
};

class Win : public Level {
public:
	Win() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		LoadFontData();
		text = Text("Win");
		text.SetPosition(ToScreenCoord({512-text.GetStringSize(), 40}));
		next = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "newMenu\\next.png", ToScreenCoord({512, 700}), 0.5f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({0.f, 0.f, 0.f, 1.f});
		text.DrawString();
		next.Draw();
		Graphics::End();
	}
	void Update() override {

	}
	void FixedUpdate() override {
		next.Update();
	}
	void UnLoad() override {
		FreeFontData();
		next.Free();
		text.Free();
	}
private:
	Text text;
	Button next;
};

class Dead : public Level {
public:
	Dead() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		button = Button([&, this]() { nLevel = Level::ManageLevel::PrevLevel; }, "MenuContent\\restart.png", ToScreenCoord({312, 384}));
		home = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "MenuContent\\homeIcon.png", ToScreenCoord({ 712, 384 }));
		LoadFontData();
		text = Text("You Died");
		text.SetPosition(ToScreenCoord({512-text.GetStringSize(), 40}));
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.f });
		text.DrawString();
		button.Draw();
		home.Draw();
		Graphics::End();
	}
	void FixedUpdate() override {
		button.Update();
		home.Update();
	}
	void Update() override {

	}
	void UnLoad() override {
		FreeFontData();
		text.Free();
		home.Free();
		button.Free();
	}
private:
	Button button;
	Button home;
	Text text;
};