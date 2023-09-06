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
		mRect = Sprite();
		w = WaveSystem(5);
		coins = Coins();
		character = std::make_unique<Character>("cIdle", "cWalk", "cHit", "cDash");
		mRect.SetTexture(Graphics::LoadTexture("map.png"), 4.0f);
		chest = Chest();
		chest.vPowerups = { Powerup("Powerups\\fastRun.png", [&, this]() {character->SetSpeed(20.f); }, [&, this]() {character->SetSpeed(10.f); }, 80000) ,
		Powerup("Powerups\\shield.png", [&, this]() {bDamageEnabled = false; }, [&, this]() {bDamageEnabled = true; }, 80000) ,
		Powerup("Powerups\\health.png", [&, this]() {character->SetHealth(300.f); }, [&, this]() {character->SetSpeed(10.f); }, 80000) };
		Graphics::SetStates();
	}
	void UnLoad() override {
		delete[] Data::fontData;
		s.UpdateFile();
		chest.Free();
		mRect.Free();
		character->Destroy();
		if (!w.enemies.empty())
			for (int i = 0; i < w.enemies.size(); i++) {
				w.enemies[i]->Destroy();
				w.enemies.erase(w.enemies.begin() + i);
			}
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
				coins.SetAmount(coins.nAmount + 1);
			}
		}
		if (!w.enemies.empty())
			for (auto& enemy : w.enemies)
				enemy->Update(character->GetPosition());
		character->Update();
		chest.Update(character->GetPosition());
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
		if (character->GetHealth() <= 0)
			nLevel = Level::ManageLevel::NextLevel;
	}
private:
	SaveFile s;
	WaveSystem w;
	std::unique_ptr<Character> character;
	Sprite mRect;
	Chest chest;
	Coins coins;
	bool bDamageEnabled = true;
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
		LoadFontData();		
		nItemIndex = 0;
		vItems = { Item("Powerups\\fastRun.png", "test 1", {100, 200, 300, 400, 500}), Item("Powerups\\health.png", "test 2", {30, 40})};
		back = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 0; }, "newMenu\\back.png", ToScreenCoord({ 920, 700 }), 0.5f);
		left = Button([&, this]() {if (nItemIndex > 0) nItemIndex--; tp = Clock::now(); }, "marketUI\\left.png", ToScreenCoord({ 64, 368 }), 1.f);
		right = Button([&, this]() {if (nItemIndex < vItems.size() - 1) nItemIndex++; tp = Clock::now(); }, "marketUI\\right.png", ToScreenCoord({ 960, 368 }), 1.f);
		buy = Button([&, this]() {vItems[nItemIndex].SetLevel(vItems[nItemIndex].nValueIndex + 1); }, "marketUI\\buy.png", ToScreenCoord({ 512, 700 }), .5f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::ClearAndBegin({ 0.f, 0.f, 0.f, 1.0f });
		back.Draw();
		vItems[nItemIndex].mIcon.Draw();
		vItems[nItemIndex].sDescription.DrawString();
		vItems[nItemIndex].sLevel.DrawString();
		left.Draw();
		right.Draw();
		buy.Draw();
		Graphics::End();
	}
	void FixedUpdate() override {
		back.Update();
		if (GetTimeLapse(Clock::now(), tp) > .5f) {
			left.Update();
			right.Update();
		}
		buy.Update();
	}
	void Update() override {
		
	}
	void UnLoad() override {
		delete[] Data::fontData;
		back.Free();
		for (auto& element : vItems)
			element.Free();
		left.Free();
		right.Free();
		buy.Free();
	}
private:
	timePoint tp;
	std::vector<Item> vItems;
	Button buy;
	Button left;
	Button right;
	Button back;
	int nItemIndex;
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
		delete[] Data::fontData;
		text.Free();
		home.Free();
		button.Free();
	}
private:
	Button button;
	Button home;
	Text text;
};