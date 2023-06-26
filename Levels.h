#include "Character.h"
#include "Enemy.h"
#include "Engine/SaveFiles.h"

class Main : public Level {
public:
	Main() {
		this->nLevel = Level::ManageLevel::CurrentLevel;
		this->ticks = 150;
	}
	void Load() override {
		s = SaveFile("NewFile.txt");
		Graphics::CreatePixelShader(pixelShader, L"Shaders\\shaders.hlsl", "ps_main");
		Graphics::CreateVertexShader(vertexShader, L"Shaders\\shaders.hlsl", "vs_main");
		mRect = Primitives::Sprite(-0.875f);
		mHealth = Primitives::Sprite(-0.1f);
		enemies.push_back(std::make_unique<Enemy>("eAttack", "eMove", "eIdle", "eDead"));
		character = std::make_unique<Character>("cIdle", "cWalk", "cHit", "cDash");
		nMap = Graphics::LoadTexture("map.png");
		nHealth = Graphics::LoadTexture("UI.gif");
		mHealth.SetPosition({ -3800.f, 3300.f });
		LoadData();
	}
	void UnLoad() override {
		s.UpdateFile();
		pixelShader.Reset();
		vertexShader.Reset();
		nMap->Release();
		mRect.Free();
		character->Destroy();
		if(!enemies.empty())
			for (auto& enemy : enemies)
				enemy->Destroy();
	}
	void FixedUpdate() override {
		for (std::size_t i = 0; i < enemies.size(); i++) {
			if (character->isState("Attack") && CheckCollision(138, 148, character->GetPosition(), enemies[i]->GetPosition()))
				enemies[i]->health -= character->nDamage;
			if(enemies[i]->health <= 0)
				enemies[i]->SetState("Dead");
			if (enemies[i]->isState("Dead") && enemies[i]->AnimEnd()) {
				enemies[i]->Destroy();
				enemies.erase(enemies.begin() + i);
				enemies.push_back(std::make_unique<Enemy>("eAttack", "eMove", "eIdle", "eDead"));
			}
		}
		if(!enemies.empty())
			for (auto& enemy : enemies) 
				enemy->Update(character->GetPosition());
		character->Update();
	}
	inline void SaveData() {
		s.OverWrite("Position: " + character->GetPosition().toString() + "\n");
		s.OverWrite("Health: " + std::to_string(character->GetHealth()) + "\n", s.GetNewLine(2));
		int nLineCount = 3;
		if (enemies.empty()) {
			s.DeleteBetween(s.GetNewLine(3), s.GetContent().size());
			return;
		}
		for (std::size_t i = 0; i < enemies.size(); ++i) {
			s.OverWrite("Enemy Position " + std::to_string(i) + ": " + enemies[i]->GetPosition().toString() + "\n", s.GetNewLine(nLineCount));
			s.OverWrite("Enemy Health " + std::to_string(i) + ": " + std::to_string(enemies[i]->health) + "\n", s.GetNewLine(nLineCount + 1));
			nLineCount += 2;
		}
	}
	inline void LoadData() {
		character->SetPosition(Math::Vec2f::toVector(s.ReadBetween(s.FindEnd("Position: "), s.GetNewLine(2) - 1)));
		character->SetHealth(atoi(s.ReadBetween(s.FindEnd("Health: "), s.GetNewLine(3) - 1).c_str()));
		if (enemies.empty()) {
			s.DeleteBetween(s.GetNewLine(3), s.GetContent().size());
			return;
		}
		int nLineCount = 4;
		for (std::size_t i = 0; i < enemies.size(); ++i) {
			enemies[i]->SetPosition(Math::Vec2f::toVector(s.ReadBetween(s.FindEnd("Enemy Position " + std::to_string(i) + ": "), s.GetNewLine(nLineCount) - 1)));
			enemies[i]->SetHealth(atoi(s.ReadBetween(s.FindEnd("Enemy Health " + std::to_string(i) + ": "), s.GetNewLine(nLineCount + 1) - 1).c_str()));
			nLineCount += 2;
		}
	}
	void Render() override {
		Graphics::MapConstantBuffer<Structures::Projection>(Camera::projBuffer, { Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix });
		FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
		Graphics::deviceContext->ClearRenderTargetView(Graphics::renderTarget.Get(), backgroundColor);
		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)Window::width, (FLOAT)Window::height, 0.0f, 1.0f };
		Graphics::deviceContext->RSSetViewports(1, &viewport);
		Graphics::deviceContext->RSSetState(Graphics::rasterizer.Get());
		Graphics::deviceContext->OMSetRenderTargets(1, Graphics::renderTarget.GetAddressOf(), nullptr);
		Graphics::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Graphics::deviceContext->IASetInputLayout(Graphics::inputLayout.Get());
		Graphics::deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		Graphics::deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
		mRect.Draw(nMap);
		mHealth.Draw(nHealth);
		character->Render();
		if (!enemies.empty())
			for (auto& enemy : enemies)
				enemy->Render();
		Graphics::deviceContext->PSSetSamplers(0, 1, Graphics::samplerState.GetAddressOf());
		Graphics::deviceContext->VSSetConstantBuffers(1, 1, &Camera::projBuffer);
		Graphics::swapChain->Present(1, 0);
	}
	uint8_t CheckCollision(float cWidth, float eWidth, Math::Vec2f cPos, Math::Vec2f ePos) {
		float radius = (cWidth + eWidth) * 3;
		Math::Vec2f vec = cPos - ePos;
		if ((radius * radius) > (vec.GetLengthSq())) return 1;
		return 0;
	}
	void Update() override {
		if (BaseStateMachine::isKeyPressed('Q'))
			SaveData();
	}
private:
	SaveFile s;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11VertexShader> vertexShader;
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::unique_ptr<Character> character;
	ID3D11ShaderResourceView* nMap;
	Primitives::Sprite mRect;
	ID3D11ShaderResourceView* nHealth;
	Primitives::Sprite mHealth;
};