#pragma once
#include "Engine/SaveSystem.h"
#include "Engine/Primitives.h"

class Enemy {
public:
	Enemy(const char* attackDir, const char* moveDir, const char* idleDir, const char* deadDir, int nWidth, int nHeight) :
		width(nWidth), height(nHeight) {
		rect = PrimitiveShapes::TexturedRect();
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
		rect.SetAttributes(position);
		rect.Draw(stateMachine.RenderState(), facingRight ? PrimitiveShapes::FlipHorizontal::FlippedHorizontal : PrimitiveShapes::FlipHorizontal::NormalHorizontal);
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
	Math::float2 position = { 5000, -100 };
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	float threshold = 150.0f, speed = 4.0f;
	AIStateMachine stateMachine;
	PrimitiveShapes::TexturedRect rect;
};