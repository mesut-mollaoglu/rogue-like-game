#include "Character.h"
#include "Enemy.h"
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
		powerup = Powerup("Powerups\\fastRun.png", [&, this]() {std::cout << "Hello World" << std::endl; }, 2000);
		chest = Chest();
		chest.vPowerups.push_back(powerup);
		//enemies.push_back(std::make_unique<Enemy>("eAttack", "eMove", "eIdle", "eDead", "eSpawn"));
		character = std::make_unique<Character>("cIdle", "cWalk", "cHit", "cDash");
		mRect.SetTexture(Graphics::LoadTexture("map.png"), 4.0f);
		LoadData();
		Graphics::SetStates();
	}
	void UnLoad() override {
		delete[] Data::fontData;
		SaveData();
		s.UpdateFile();
		chest.Free();
		mRect.Free();
		character->Destroy();
		if (!enemies.empty())
			for (int i = 0; i < enemies.size(); i++) {
				enemies[i]->Destroy();
				enemies.erase(enemies.begin() + i);
			}
	}
	void FixedUpdate() override {
		for (std::size_t i = 0; i < enemies.size(); i++) {
			if (character->isState("Dash") && CheckCollision(138, 148, character->GetPosition(), enemies[i]->GetPosition()))
				enemies[i]->health = 0;
			if (character->isState("Attack") && CheckCollision(138, 148, character->GetPosition(), enemies[i]->GetPosition()))
				enemies[i]->health -= character->nDamage;
			if (enemies[i]->isState("Attack") && CheckCollision(138, 148, character->GetPosition(), enemies[i]->GetPosition()))
				character->SetHealth(character->GetHealth() - enemies[i]->nDamage);
			if (enemies[i]->health <= 0)
				enemies[i]->SetState("Dead");
			if (enemies[i]->isState("Dead") && enemies[i]->AnimEnd()) {
				enemies[i]->Destroy();
				enemies.erase(enemies.begin() + i);
				enemies.push_back(std::make_unique<Enemy>("eAttack", "eMove", "eIdle", "eDead", "eSpawn"));
			}
		}
		if (!enemies.empty())
			for (auto& enemy : enemies)
				enemy->Update(character->GetPosition());
		character->Update();
		chest.Update(character->GetPosition());
	}
	void Render() override {
		Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, { Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		FLOAT backgroundColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		Graphics::deviceContext->ClearRenderTargetView(Graphics::renderTarget.Get(), backgroundColor);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)Window::width, (FLOAT)Window::height, 0.0f, 1.0f };
		Graphics::deviceContext->RSSetViewports(1, &viewport);
		mRect.Draw();
		chest.Render(character->GetPosition());
		character->Render();
		if (!enemies.empty())
			for (auto& enemy : enemies)
				enemy->Render();
		character->RenderHealth();
		Graphics::swapChain->Present(1, 0);
	}
	uint8_t CheckCollision(float cWidth, float eWidth, Vec2f cPos, Vec2f ePos) {
		float radius = (cWidth + eWidth) * 3;
		Vec2f vec = cPos - ePos;
		if ((radius * radius) > (vec.GetLengthSq())) return 1;
		return 0;
	}
	uint8_t GetPowerup() {

	}
	inline void SaveData() {
		s.OverWrite("Position: " + character->GetPosition().toString() + "\n");
		s.OverWrite("Health: " + std::to_string(character->GetHealth()) + "\n", s.GetNewLine(2));
		s.OverWrite("State: " + std::string(character->GetState()) + "\n", s.GetNewLine(3));
		s.OverWrite("Direction: " + std::string(character->facingRight ? "Right" : "Left") + "\n", s.GetNewLine(4));
		int nLineCount = 5;
		if (enemies.empty()) {
			s.DeleteBetween(s.GetNewLine(3), s.GetContent().size());
			return;
		}
		for (std::size_t i = 0; i < enemies.size(); ++i) {
			s.OverWrite("Enemy Position " + std::to_string(i) + ": " + enemies[i]->GetPosition().toString() + "\n", s.GetNewLine(nLineCount));
			s.OverWrite("Enemy Health " + std::to_string(i) + ": " + std::to_string(enemies[i]->health) + "\n", s.GetNewLine(nLineCount + 1));
			s.OverWrite("Enemy State " + std::to_string(i) + ": " + std::string(enemies[i]->GetState()) + "\n", s.GetNewLine(nLineCount + 2));
			s.OverWrite("Enemy Direction " + std::to_string(i) + ": " + std::string(enemies[i]->facingRight ? "Right" : "Left") + "\n", s.GetNewLine(nLineCount + 3));
			nLineCount += 4;
		}
	}
	inline void LoadData() {
		if (s.GetContent().empty() || atoi(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(3) - 1).c_str()) <= 0)
		{
			character->SetHealth(300);
			character->SetPosition(Vec2f(-4400, 2000));
			character->SetState("Idle");
			character->facingRight = false;
			return;
		}
		character->SetPosition(Vec2f::toVector(s.ReadBetween(s.FindEnd("Position: "), s.GetNewLine(2) - 1)));
		character->SetHealth(atoi(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(3) - 1).c_str()));
		character->SetState(s.ReadBetween(s.FindEnd("State: "), s.GetNewLine(4) - 1));
		std::string direction = s.ReadBetween(s.FindEnd("Direction: "), s.GetNewLine(5) - 1);
		character->facingRight = strcmp(direction.c_str(), "Right") == 0 ? 1 : 0;
		if (enemies.empty()) {
			s.DeleteBetween(s.GetNewLine(3), s.GetContent().size());
			return;
		}
		int nLineCount = 6;
		for (std::size_t i = 0; i < enemies.size(); ++i) {
			enemies[i]->SetPosition(Vec2f::toVector(s.ReadBetween(s.FindEnd("Enemy Position " + std::to_string(i) + ": "), s.GetNewLine(nLineCount) - 1)));
			enemies[i]->SetHealth(atoi(s.ReadBetween(s.FindEnd("Enemy Health " + std::to_string(i) + ": "), s.GetNewLine(nLineCount + 1) - 1).c_str()));
			enemies[i]->SetState(s.ReadBetween(s.FindEnd("Enemy State " + std::to_string(i) + ": "), s.GetNewLine(nLineCount + 2) - 1));
			std::string direction = s.ReadBetween(s.FindEnd("Enemy Direction " + std::to_string(i) + ": "), s.GetNewLine(nLineCount + 3) - 1).c_str();
			enemies[i]->facingRight = strcmp(direction.c_str(), "Right") == 0 ? 1 : 0;
			nLineCount += 4;
		}
	}
	void Update() override {
		if (isKeyPressed('Q'))
			SaveData();
		if (character->GetHealth() <= 0)
			nLevel = Level::ManageLevel::NextLevel;
	}
private:
	SaveFile s;
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::unique_ptr<Character> character;
	Sprite mRect;
	Chest chest;
	Powerup powerup;
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
		sprite.SetTexture(Graphics::LoadTexture("MenuContent\\credits_screen.png"), 3.0f);
	}
	void Render() override {
		Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, { Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		FLOAT backgroundColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		Graphics::deviceContext->ClearRenderTargetView(Graphics::renderTarget.Get(), backgroundColor);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)Window::width, (FLOAT)Window::height, 0.0f, 1.0f };
		Graphics::deviceContext->RSSetViewports(1, &viewport);
		sprite.Draw();
		Graphics::swapChain->Present(1, 0);
	}
	void FixedUpdate() override {
		if (isKeyPressed(VK_BACK)) {
			nLevel = Level::ManageLevel::GotoLevel;
			nIndex = 0;
		}
	}
	void Update() override {

	}
	void UnLoad() override {
		sprite.Free();
	}
private:
	Sprite sprite;
};

class MainMenu : public Level {
public:
	MainMenu() {
		nLevel = Level::ManageLevel::CurrentLevel;
		ticks = 150;
	}
	void Load() override {
		start = Button([&, this]() { nLevel = Level::ManageLevel::NextLevel; }, "MenuContent\\start.png", ToScreenCoord({ 512, 300 }), 0.5f);
		credits = Button([&, this]() { nLevel = Level::ManageLevel::GotoLevel; nIndex = 3; }, "MenuContent\\credits.png", ToScreenCoord({ 512, 375 }), 0.5f);
		exit = Button([&, this]() { PostQuitMessage(0); }, "MenuContent\\exit.png", ToScreenCoord({ 512, 450 }), 0.5f);
		sprite = Sprite();
		sprite.SetTexture(Graphics::LoadTexture("MenuContent\\main menu.png"), 4.0f);
		Graphics::SetStates();
	}
	void Render() override {
		Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, { Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		FLOAT backgroundColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		Graphics::deviceContext->ClearRenderTargetView(Graphics::renderTarget.Get(), backgroundColor);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)Window::width, (FLOAT)Window::height, 0.0f, 1.0f };
		Graphics::deviceContext->RSSetViewports(1, &viewport);
		sprite.Draw();
		start.Draw();
		credits.Draw();
		exit.Draw();
		Graphics::swapChain->Present(1, 0);
	}
	void FixedUpdate() override {
		start.Update();
		credits.Update();
		exit.Update();
	}
	void Update() override {

	}
	void UnLoad() override {
		start.Free();
		credits.Free();
		exit.Free();
		sprite.Free();
	}
private:
	Button start;
	Button credits;
	Button exit;
	Sprite sprite;
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
		Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, { Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		FLOAT backgroundColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		Graphics::deviceContext->ClearRenderTargetView(Graphics::renderTarget.Get(), backgroundColor);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)Window::width, (FLOAT)Window::height, 0.0f, 1.0f };
		Graphics::deviceContext->RSSetViewports(1, &viewport);
		text.DrawString();
		button.Draw();
		home.Draw();
		Graphics::swapChain->Present(1, 0);
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