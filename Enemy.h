#pragma once
#include "Engine/Primitives.h"

class Enemy {
public:
	Enemy(std::string attackDir, std::string moveDir, std::string idleDir, std::string deadDir) {
		rect = Primitives::Sprite();
		stateMachine.AddState(Follow, new Animator(Graphics::LoadFromDir(moveDir), 250), "Follow");
		stateMachine.AddState(Idle, new Animator(Graphics::LoadFromDir(idleDir), 250), "Idle");
		stateMachine.AddState(Attack, new Animator(Graphics::LoadFromDir(attackDir), 75, true), "Attack");
		stateMachine.AddState(Dead, new Animator(Graphics::LoadFromDir(deadDir), 250, true), "Dead");
	}
	std::function<void()> Dead = [&, this]() {};
	std::function<void()> Follow = [&, this]() {
		float angle = Math::GetAngle(position, characterPosition);
		float t = elapsedTime * speed;
		t = Math::smoothstep(0.0f, 20.0f, t);
		position -= Math::toVector(angle) * t * 10.0f;
	};
	std::function<void()> Attack = [&, this]() {};
	std::function<void()> Idle = [&, this]() {};
	void Render() {
		rect.SetPosition(GetPosition());
		rect.Draw(stateMachine.RenderState(), facingRight ? Primitives::FlipHorizontal::FlippedHorizontal : Primitives::FlipHorizontal::NormalHorizontal);
	}
	void Update(Math::Vec2f pos) {
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
	float health = 50.f;
	float width, height;
	bool facingRight = false;
	Math::Vec2f GetPosition() {
		return position;
	}
	void Destroy() {
		stateMachine.Clear();
		rect.Free();
	}
	void SetState(std::string state) {
		stateMachine.SetState(state);
	}
	bool AnimEnd() {
		assert(stateMachine.GetCurrentState().mAnimator->ShouldPlayOnce());
		return stateMachine.GetCurrentState().mAnimator->GetIndex() == stateMachine.GetCurrentState().mAnimator->GetSize() - 2;
	}
	bool isState(std::string state) {
		return stateMachine.equals(state);
	}
	void SetPosition(Math::Vec2f fPos) {
		position = fPos;
	}
	void SetHealth(float fHealth) {
		health = fHealth;
	}
	virtual ~Enemy(){}
private:
	Math::Vec2f characterPosition;
	Math::Vec2f position = { 0, -800 };
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	float threshold = 150.0f, speed = 4.0f;
	AIStateMachine stateMachine;
	Primitives::Sprite rect;
};