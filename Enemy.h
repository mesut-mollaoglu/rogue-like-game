#pragma once
#include "SaveSystem.h"

class Enemy {
public:
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;
	Enemy(const char* attackDir, const char* moveDir, const char* idleDir, const char* deadDir, int nWidth, int nHeight) :
		width(nWidth), height(nHeight) {
		float aspectRatio = (float)width / (float)height;
		Graphics::Vertex vertices[] =
		{
			XMFLOAT2(-0.875 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 1),
			XMFLOAT2(-0.875 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 0),
			XMFLOAT2(0.875 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 0),
			XMFLOAT2(0.875 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 1)
		};
		Graphics::CreateVertexBuffer(vertexBuffer, vertices, ARRAYSIZE(vertices));
		Graphics::CreateIndexBuffer(indexBuffer);
		Graphics::CreateConstantBuffer<Graphics::Constants>(constantBuffer);
		stateMachine.AddState(Follow, new Animator(Sprite::LoadFromDir(moveDir, width, height), 250), "Follow");
		stateMachine.AddState(Idle, new Animator(Sprite::LoadFromDir(idleDir, width, height), 250), "Idle");
		stateMachine.AddState(Attack, new Animator(Sprite::LoadFromDir(attackDir, width, height), 75, true), "Attack");
	}
	std::function<void()> Follow = [&, this]() {
		float angle = Math::GetAngle(position, characterPosition);
		float t = elapsedTime * speed;
		t = Math::smoothstep(0.0f, 20.0f, t);
		position -= Math::toVector(angle) * t * 10.0f;
	};
	std::function<void()> Attack = [&, this]() {};
	std::function<void()> Idle = [&, this]() {};
	void Render() {
		Graphics::SetConstantValues<Graphics::Constants>(constantBuffer.Get(), {
			XMFLOAT2{ (this->GetPosition().x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
			(this->GetPosition().y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight() },
			XMFLOAT2{ 0, 0 }, XMFLOAT4{ (this->facingRight) ? -1.0f : 1.0f, 0, 0, 0} });
		ID3D11ShaderResourceView* currentFrame = stateMachine.RenderState();
		Graphics::d3dDeviceContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		Graphics::d3dDeviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		Graphics::d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		Graphics::d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &Graphics::stride,
			&Graphics::offset);
		Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &currentFrame);
		Graphics::d3dDeviceContext->DrawIndexed(6, 0, 0);
	}
	void Update(Math::float2 pos) {
		characterPosition = pos;
		float distance = Math::abs(characterPosition.GetDistance(position));
		if (distance < 7000.0f && distance > 550.0f) stateMachine.SetState("Follow");
		else if (distance >= 7000.0f) stateMachine.SetState("Idle");
		else if (distance <= 550.0f) stateMachine.SetState("Attack");
		this->elapsedTime += 0.01f;
		if (!stateMachine.equals("Attack")) {
			position.y += this->smoothSin(this->elapsedTime, 1.5f, 6.0f);
			m_time += m_deltaTime;
		}
		this->facingRight = (position.x > characterPosition.x) ? true : false;
		stateMachine.UpdateState();
	}
	float health = 20.0f;
	float width, height;
	bool facingRight = false;
	Math::float2 GetPosition() {
		return position;
	}
private:
	Math::float2 characterPosition;
	Math::float2 position = { 2000, -100 };
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	float threshold = 150.0f, speed = 4.0f;
	AIStateMachine stateMachine;
};