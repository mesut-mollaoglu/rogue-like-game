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
		s = SaveFile("NewFile.txt");
		mRect = Sprite();
		coins = Coins();
		LoadData();
		w = WaveSystem(nMaxWaves);
		mRect.SetTexture(Graphics::LoadTexture("Assets\\Misc\\map.png"), 4.5f);
		chest = Chest();
		chest.mFunction = [&, this]() {if (GetTimeLapse(Clock::now(), tp) < .5f || coins.nAmount < 2) return; coins.SetAmount(coins.nAmount - 2);  tp = Clock::now(); };
		character = std::make_unique<Character>(nHealth);
		character->SetSpeed(nSpeed);
		character->SetHealth(nHealth);
		chest.vPowerups = { Powerup("Assets\\Chest\\Powerups\\fastRun.png", [&, this]() {character->SetSpeed(nSpeed * 2); }, [&, this]() {character->SetSpeed(nSpeed); }, 80000) ,
		Powerup("Assets\\Chest\\Powerups\\shield.png", [&, this]() {bDamageEnabled = false; }, [&, this]() {bDamageEnabled = true; }, 80000) ,
		Powerup("Assets\\Chest\\Powerups\\health.png", [&, this]() {character->SetHealth(nHealth); }, [&, this]() {character->SetSpeed(nSpeed); }, 80000) };
		paused = Text("Paused");
		paused.SetPosition(ToScreenCoord({ 512 - paused.GetStringSize(), 40 }));
		home = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "Assets\\UI\\Menu\\homeIcon.png", ToScreenCoord({ 312, 384 }));
		resume = Button([&, this]() {gameState = GameState::GameWorking; }, "Assets\\UI\\Menu\\resumeIcon.png", ToScreenCoord({ 712, 384 }));
		Graphics::SetStates();
		gameState = GameState::GameWorking;
	}
	void FixedUpdate() override {
		switch (gameState) {
		case GameState::GamePaused: {
			resume.Update();
			home.Update();
		}
								  break;
		case GameState::GameWorking: {
			for (int i = 0; i < w.enemies.size(); i++) {
				w.enemies[i]->Update(character->GetPosition());
				if (character->isState("Dash") && CheckCollision(w.enemies[i].get()))
					w.enemies[i]->health = 0;
				if (character->isState("Attack") && CheckCollision(w.enemies[i].get()))
					w.enemies[i]->health -= character->nDamage;
				if (bDamageEnabled && w.enemies[i]->nDamage != 0.f && !character->isState("Dash"))
					character->SetHealth(character->GetHealth() - w.enemies[i]->nDamage);
				if (w.enemies[i]->bDead && !w.enemies.empty()) {
					w.enemies[i]->Destroy();
					w.enemies.erase(w.enemies.begin() + i);
					coins.SetAmount(coins.nAmount + nCoinIncrement);
				}
			}
			character->Update();
			chest.Update(coins.nAmount >= 2, character->GetPosition());
			if (w.bFinished) {
				nLevel = Level::ManageLevel::GotoLevel;
				nIndex = 5;
			}
			w.Update();
		}
								   break;
		}
	}
	void Render() override { 
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		switch (gameState) {
		case GameState::GamePaused: {
			home.Draw();
			resume.Draw();
			paused.DrawString();
		}
								  break;
		case GameState::GameWorking: {
			std::sort(std::begin(w.enemies), std::end(w.enemies), [&, this]
			(const auto& e1, const auto &e2) {
					return e1->GetPosition().y > e2->GetPosition().y;
				});
			mRect.Draw();
			chest.Render(character->GetPosition());
			w.Render();
			coins.Render();
			if (!w.enemies.empty())
				for (auto& enemy : w.enemies)
					if (enemy->GetPosition().y > character->GetPosition().y)
						enemy->Render();
			character->Render();
			if (!w.enemies.empty())
				for (auto& enemy : w.enemies)
					if(enemy->GetPosition().y <= character->GetPosition().y)
						enemy->Render();
			character->RenderHealth();
		}
								   break;
		}
		Graphics::End();
	}
	uint8_t CheckCollision(Entity* enemy) {
		float distance = enemy->GetPosition().GetDistance(character->GetPosition());
		bool facing = (character->GetPosition().x < enemy->GetPosition().x && !character->facingRight) || (character->GetPosition().x >= enemy->GetPosition().x && character->facingRight);
		return (uint8_t)(facing && distance < 1400.f);
	}
	void Update() override {
		switch (gameState) {
		case GameState::GameWorking: {
			if (isKeyPressed(VK_ESCAPE))
				gameState = GameState::GamePaused;
			if (character->GetHealth() <= 0)
				nLevel = Level::ManageLevel::NextLevel;
		}
								   break;
		case GameState::GamePaused: {

		}
								  break;
		}
	}
	inline void SaveData() {
		if (s.isEmpty()) {
			s.OverWrite("Speed: " + std::to_string(character->GetSpeed()) + "\n");
			s.OverWrite("Health: " + std::to_string(character->GetHealth()) + "\n", s.GetNewLine(1));
			s.OverWrite("Increment: " + std::to_string((int)nCoinIncrement) + "\n", s.GetNewLine(2));
			s.OverWrite("Max Waves: " + std::to_string((int)w.nMaxNumber) + "\n", s.GetNewLine(3));
			s.OverWrite("Coins: " + std::to_string(coins.nAmount) + "\n", s.GetNewLine(4));
			return;
		}
		s.OverWrite("Coins: " + std::to_string(coins.nAmount) + "\n", s.GetNewLine(4));
	}
	inline void LoadData() {
		if (s.isEmpty()) {
			nSpeed = 10.f;
			nHealth = 300.f;
			nCoinIncrement = 1;
			nMaxWaves = 5;
			coins.SetAmount(0);
			return;
		}
		nSpeed = atoi(s.ReadBetween(s.FindEnd("Speed: "), s.GetNewLine(2) - 1).c_str());
		nHealth = atoi(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(3) - 1).c_str());
		nCoinIncrement = atoi(s.ReadBetween(s.FindEnd("Increment: "), s.GetNewLine(4) - 1).c_str());
		nMaxWaves = atoi(s.ReadBetween(s.FindEnd("Max Waves: "), s.GetNewLine(5) - 1).c_str());
		coins.SetAmount(atoi(s.ReadBetween(s.FindEnd("Coins: "), s.GetNewLine(6) - 1).c_str()));
	}
	void UnLoad() override {
		SaveData();
		s.UpdateFile();
		chest.Free();
		mRect.Free();
		character->Destroy();
		character.reset();
		w.Free();
		coins.Free();
		paused.Free();
		home.Free();
		resume.Free();
	}
private:
	enum class GameState {
		GamePaused,
		GameWorking
	}gameState = GameState::GameWorking;
	SaveFile s;
	WaveSystem w;
	std::unique_ptr<Character> character;
	Sprite mRect;
	Chest chest;
	Coins coins;
	bool bDamageEnabled = true;
	float nSpeed, nHealth;
	int nCoinIncrement, nMaxWaves;
	timePoint tp;
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
		back = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "Assets\\UI\\Menu\\back.png", ToScreenCoord({ 920, 700 }), 0.5f);
		sprite.SetTexture(Graphics::LoadTexture("Assets\\UI\\Menu\\credits_screen.png"), 3.0f);
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
		start = Button([&, this]() { nLevel = Level::ManageLevel::NextLevel; }, "Assets\\UI\\Menu\\start.png", ToScreenCoord({ 512, 300 }), 0.5f);
		credits = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 3; }, "Assets\\UI\\Menu\\credits.png", ToScreenCoord({ 512, 375 }), 0.5f);
		market = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 4; }, "Assets\\UI\\Menu\\market.png", ToScreenCoord({ 512, 450 }), 0.5f);
		exit = Button([&, this]() { PostQuitMessage(0); }, "Assets\\UI\\Menu\\exit.png", ToScreenCoord({ 512, 525 }), 0.5f);
		sprite = Sprite();
		sprite.SetTexture(Graphics::LoadTexture("Assets\\UI\\Menu\\main menu.png"), 4.0f);
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
		nItemIndex = 0;
		c = Coins();
		vItems = { Item("Assets\\Chest\\Powerups\\fastRun.png", "Increases Walking Speed.", {10, 15, 20, 25}),
			Item("Assets\\Chest\\Powerups\\health.png", "Increases Maximum Health.", {300, 400, 500, 600}),
			Item("Assets\\Chest\\Powerups\\moneyIcon.png", "Amount of money you earn by killing enemies.", {1, 2, 3, 4, 5}),
			Item("Assets\\Chest\\Powerups\\waveIcon.png", "Max number of waves.", {5, 8, 11, 14, 17}) };
		LoadData();
		back = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "Assets\\UI\\Menu\\back.png", ToScreenCoord({ 920, 700 }), 0.5f);
		left = Button([&, this]() {if (GetTimeLapse(Clock::now(), tp1) < .5f) return; if (nItemIndex > 0) nItemIndex--; tp1 = Clock::now(); }, "Assets\\UI\\Market\\left.png", ToScreenCoord({ 64, 368 }), 1.f);
		right = Button([&, this]() {if (GetTimeLapse(Clock::now(), tp1) < .5f) return; if (nItemIndex < vItems.size() - 1) nItemIndex++; tp1 = Clock::now(); }, "Assets\\UI\\Market\\right.png", ToScreenCoord({ 960, 368 }), 1.f);
		buy = Button([&, this]() {if (GetTimeLapse(Clock::now(), tp2) < .5f || c.nAmount < (2 << vItems[nItemIndex].nValueIndex)) return;
		if (vItems[nItemIndex].vValues.size() > (uint64_t)vItems[nItemIndex].nValueIndex + 1) c.SetAmount(c.nAmount - (2 << vItems[nItemIndex].nValueIndex)); vItems[nItemIndex].SetLevel(vItems[nItemIndex].nValueIndex + 1);
		tp2 = Clock::now(); }, "Assets\\UI\\Market\\buy.png", ToScreenCoord({ 512, 700 }), .5f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		back.Draw();
		vItems[nItemIndex].mIcon.Draw();
		vItems[nItemIndex].sDescription.DrawString();
		vItems[nItemIndex].sLevel.DrawString();
		if (vItems[nItemIndex].vValues.size() > (uint64_t)vItems[nItemIndex].nValueIndex + 1)
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
		vItems[0].SetLevel(vItems[0].FindValue(atof(s.ReadBetween(s.FindEnd("Speed: "), s.GetNewLine(1) - 1).c_str())));
		vItems[1].SetLevel(vItems[1].FindValue(atof(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(2) - 1).c_str())));
		vItems[2].SetLevel(vItems[2].FindValue(atoi(s.ReadBetween(s.FindEnd("Increment: "), s.GetNewLine(3) - 1).c_str())));
		vItems[3].SetLevel(vItems[3].FindValue(atoi(s.ReadBetween(s.FindEnd("Max Waves: "), s.GetNewLine(4) - 1).c_str())));
		c.SetAmount(atoi(s.ReadBetween(s.FindEnd("Coins: "), s.GetNewLine(5) - 1).c_str()));
	}
	inline void SaveData() {
		s.OverWrite("Speed: " + std::to_string(vItems[0].GetCurrentValue()) + "\n");
		s.OverWrite("Health: " + std::to_string(vItems[1].GetCurrentValue()) + "\n", s.GetNewLine(1));
		s.OverWrite("Increment: " + std::to_string((int)vItems[2].GetCurrentValue()) + "\n", s.GetNewLine(2));
		s.OverWrite("Max Waves: " + std::to_string((int)vItems[3].GetCurrentValue()) + "\n", s.GetNewLine(3));
		s.OverWrite("Coins: " + std::to_string(c.nAmount) + "\n", s.GetNewLine(4));
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
		text = Text("Win");
		text.SetPosition(ToScreenCoord({ 512 - text.GetStringSize(), 40 }));
		next = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "Assets\\UI\\Menu\\next.png", ToScreenCoord({ 512, 700 }), 0.5f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.f });
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
		button = Button([&, this]() { nLevel = Level::ManageLevel::PrevLevel; }, "Assets\\UI\\Menu\\restart.png", ToScreenCoord({ 312, 384 }));
		home = Button([&, this]() {nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "Assets\\UI\\Menu\\homeIcon.png", ToScreenCoord({ 712, 384 }));
		text = Text("You Died");
		text.SetPosition(ToScreenCoord({ 512 - text.GetStringSize(), 40 }));
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
		text.Free();
		home.Free();
		button.Free();
	}
private:
	Button button;
	Button home;
	Text text;
};