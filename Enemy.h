#pragma once
#include "Sprite.h"

using namespace DirectX;

class Enemy {
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	struct Vertex {
		XMFLOAT2 pos;
		XMFLOAT4 color;
		XMFLOAT2 tex;
	};
	Enemy(const char* attackDir, const char* moveDir, const char* idleDir, const char* deadDir, int nWidth, int nHeight, float posX, float posY, Sprite* sprite, Graphics* graphics) : spriteLoader(sprite),
		width(nWidth), height(nHeight), x(posX), y(posY), gfx(graphics) {
		float aspectRatio = (float)width / (float)height;
		Vertex OurVertices[] =
		{
			XMFLOAT2(-0.875 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 1),
			XMFLOAT2(-0.875 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 0),
			XMFLOAT2(0.875 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 0),
			XMFLOAT2(0.875 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 1)
		};
		DWORD indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		D3D11_BUFFER_DESC indexBufferDesc = { 0 };
		indexBufferDesc.ByteWidth = sizeof(indices) * ARRAYSIZE(indices);
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA indexSubData = { indices, 0, 0 };
		this->gfx->d3dDevice.Get()->CreateBuffer(&indexBufferDesc, &indexSubData, this->indexBuffer.GetAddressOf());
		D3D11_BUFFER_DESC bd = { 0 };
		bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(OurVertices);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA srd = { OurVertices, 0, 0 };
		this->gfx->d3dDevice->CreateBuffer(&bd, &srd, this->vertexBuffer.GetAddressOf());
		this->enemyAttack = this->spriteLoader->LoadFromDir(attackDir, width, height);
		this->enemyMoving = this->spriteLoader->LoadFromDir(moveDir, width, height);
		this->enemyIdle = this->spriteLoader->LoadFromDir(idleDir, width, height);
		this->enemyDead = this->spriteLoader->LoadFromDir(deadDir, width, height);
		this->isSpawning = true;
		states = Spawn;
	}
	void Render() {
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
	void Update(float character_x, float character_y) {
		
		if (isDead) {
			enemyAttack.clear();
			enemyMoving.clear();
			enemyIdle.clear();
			enemyDead.clear();
			return;
		}
		this->elapsedTime += 0.01f;
		if (!attacking) {
			this->y += this->smoothSin(this->elapsedTime, 1.5f, 6.0f);
			m_time += m_deltaTime;
		}
		this->FollowTarget(character_x, character_y, this->elapsedTime);
		this->facingRight = (this->x > character_x) ? true : false;
		frame++;		
	}

	void FollowTarget(float character_x, float character_y, float elapsedTime) {
		DirectX::XMFLOAT2 playerPos = DirectX::XMFLOAT2(character_x, character_y);
		DirectX::XMFLOAT2 enemyPos = DirectX::XMFLOAT2(this->x, this->y);
		DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&playerPos),
			DirectX::XMLoadFloat2(&enemyPos));
		float distance_x = XMVectorGetX(XMVector2Length(delta));
		float distance_y = XMVectorGetY(XMVector2Length(delta));
		float distance = sqrt(distance_x * distance_x + distance_y * distance_y);
		if (abs(distance) < 7000.0f && abs(distance) > 550.0f && !this->attacking) {
			this->moving = true;
			this->attacking = false;
			this->idle = false;
			DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&playerPos),
				DirectX::XMLoadFloat2(&enemyPos));
			direction = DirectX::XMVector2Normalize(direction);
			float t = elapsedTime * speed;
			t = smoothstep(0.0f, 20.0f, t);
			this->x += direction.m128_f32[0] * t * 10.0f;
			this->y += direction.m128_f32[1] * t * 10.0f;
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
	float x, y;
	ID3D11ShaderResourceView* currentFrame;
	float health = 20.0f;
	bool attacking;
	float width, height;
	int hitFrame = 0;
	bool damageEnabled = true, isDead;
	bool facingRight = false;
	bool isSpawning = false;
	enum EnemyStates {
		Spawn,
		Idle,
		Attacking,
		Following,
		Dead
	};
	EnemyStates states;
private:
	Graphics* gfx;
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