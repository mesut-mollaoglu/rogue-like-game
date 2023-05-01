#pragma once
#include "StateMachine.h"
#include "Sprite.h"

using namespace DirectX;

class Enemy {
public:
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;
	Enemy(const char* attackDir, const char* moveDir, const char* idleDir, const char* deadDir, int nWidth, int nHeight, Sprite* sprite) : spriteLoader(sprite),
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
		this->enemyAttack = this->spriteLoader->LoadFromDir(attackDir, width, height);
		this->enemyMoving = this->spriteLoader->LoadFromDir(moveDir, width, height);
		this->enemyIdle = this->spriteLoader->LoadFromDir(idleDir, width, height);
		this->enemyDead = this->spriteLoader->LoadFromDir(deadDir, width, height);
		this->isSpawning = true;
	}
	void Render() {
		Graphics::SetConstantValues<Graphics::Constants>(constantBuffer.Get(), {
			XMFLOAT2{ (this->GetPosition().x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(), 
			(this->GetPosition().y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight() },
			XMFLOAT2{ 0, 0 }, XMFLOAT4{ (this->facingRight) ? -1.0f : 1.0f, 0, 0, 0} });
		if (this->moving) this->currentFrame = this->enemyMoving[(this->frame / 24) % 2];
		if (this->attacking) {
			if (this->hitFrame < 81) {
				this->currentFrame = this->enemyAttack[this->hitFrame / 9];
				hitFrame++;
				isHitting = true;
			}
			else{
				isHitting = false;
				this->hitFrame = 0;
				attacking = false;
				damageEnabled = true;
			}
		}
		if (this->idle) {
			this->currentFrame = this->enemyIdle[(this->frame / 24) % 2];
		}
		else if(health <= 0) {
			if (this->deadFrame < 90) {
				this->currentFrame = this->enemyDead[deadFrame / 10];
				deadFrame++;
			}
			if (this->deadFrame == 90) isDead = true;
		}
	}
	void Update(Math::float2 pos) {
		
		if (isDead) {
			enemyAttack.clear();
			enemyMoving.clear();
			enemyIdle.clear();
			enemyDead.clear();
			return;
		}
		this->elapsedTime += 0.01f;
		if (!attacking) {
			position.y += this->smoothSin(this->elapsedTime, 1.5f, 6.0f);
			m_time += m_deltaTime;
		}
		this->FollowTarget(pos, this->elapsedTime);
		this->facingRight = (position.x > pos.x) ? true : false;
		frame++;		
	}

	void FollowTarget(Math::float2 pos, float elapsedTime) {
		float distance = pos.GetDistance(position);
		if (abs(distance) < 7000.0f && abs(distance) > 550.0f && !this->attacking) {
			this->moving = true;
			this->attacking = false;
			this->idle = false;
			float angle = Math::GetAngle(position, pos);
			float t = elapsedTime * speed;
			t = smoothstep(0.0f, 20.0f, t);
			position -= Math::toVector(angle) * t * 10.0f;
		}
		else if (abs(distance) > 7000.0f) {
			this->moving = false;
			this->attacking = false;
			this->idle = true;
		}
		else {
			this->moving = false;
			this->attacking = true;
			this->idle = false;
		}
	}
	ID3D11ShaderResourceView* currentFrame;
	float health = 20.0f;
	bool attacking;
	float width, height;
	int hitFrame = 0;
	bool damageEnabled = true, isDead;
	bool facingRight = false;
	bool isSpawning = false;
	Math::float2 GetPosition() {
		return position;
	}
private:
	Math::float2 characterPosition;
	Math::float2 position = {2000, -100};
	std::vector<ID3D11ShaderResourceView*> enemyAttack;
	std::vector<ID3D11ShaderResourceView*> enemyMoving;
	std::vector<ID3D11ShaderResourceView*> enemyIdle;
	std::vector<ID3D11ShaderResourceView*> enemyDead;
	int frame = 0, deadFrame = 0;
	bool idle, moving;
	const float PI = 3.14159265f;
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float smoothstep(float edge0, float edge1, float x) {
		x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return x * x * (3.0f - 2.0f * x);
	};
	float clamp(float x, float min, float max) {
		return fmin(fmax(x, min), max);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	Sprite* spriteLoader;
	float threshold = 150.0f, speed = 4.0f;
	bool isHitting, isMoving;
};