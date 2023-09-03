#pragma once
#include "Engine/Primitives.h"

class Enemy {
public:
	Enemy(std::string attackDir, std::string moveDir, std::string idleDir, std::string deadDir, std::string spawnDir) {
		rect = Sprite();
		stateMachine.AddState(Spawn, new Animator(Graphics::LoadFromDir(spawnDir), 125, true), "Spawn");
		stateMachine.AddState(Follow, new Animator(Graphics::LoadFromDir(moveDir), 250), "Follow");
		stateMachine.AddState(Idle, new Animator(Graphics::LoadFromDir(idleDir), 250), "Idle");
		stateMachine.AddState(Attack, new Animator(Graphics::LoadFromDir(attackDir), 75, true), "Attack");
		stateMachine.AddState(Dead, new Animator(Graphics::LoadFromDir(deadDir), 250, true), "Dead");
		stateMachine.SetState("Spawn");
	}
	std::function<void()> Spawn = [&, this]() {
		attackStates = AttackStates::DamagePlayer;
	};
	std::function<void()> Dead = [&, this]() {};
	std::function<void()> Follow = [&, this]() {
		attackStates = AttackStates::DamagePlayer;
		float angle = GetAngle(position, characterPosition);
		float t = elapsedTime * speed;
		t = Smoothstep(0.0f, 20.0f, t);
		position -= toVector(angle) * t * 10.0f;
	};
	std::function<void()> Attack = [&, this]() {
		switch (attackStates) {
		case AttackStates::DamagePlayer: {
			nDamage = 10.f;
			attackStates = AttackStates::Cooldown;
		}
									  break;
		case AttackStates::Cooldown: {
			nDamage = 0.f;
			if (AnimEnd()) attackStates = AttackStates::DamagePlayer;
		}
								   break;
		}};
	std::function<void()> Idle = [&, this]() {attackStates = AttackStates::DamagePlayer; };
	void Render() {
		rect.SetPosition(GetPosition());
		rect.SetTexture(stateMachine.RenderState());
		rect.Draw(facingRight ? FlipHorizontal::FlippedHorizontal : FlipHorizontal::NormalHorizontal);
	}
	void Update(Vec2f pos) {
		characterPosition = pos;
		float distance = Abs(characterPosition.GetDistance(position));
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
	Vec2f GetPosition() {
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
		assert(stateMachine.GetCurrentState().mAnimator->shouldPlayOnce());
		return stateMachine.GetCurrentState().mAnimator->GetIndex() == stateMachine.GetCurrentState().mAnimator->GetSize() - 2;
	}
	bool isState(std::string state) {
		return stateMachine.equals(state);
	}
	void SetPosition(Vec2f fPos) {
		position = fPos;
	}
	void SetHealth(float fHealth) {
		health = fHealth;
	}
	const char* GetState() {
		return stateMachine.GetState();
	}
	virtual ~Enemy(){}
	enum class AttackStates {
		DamagePlayer,
		Cooldown,
	}attackStates = AttackStates::DamagePlayer;
	float nDamage = 0.f;
private:
	Vec2f characterPosition;
	Vec2f position = { 0, -800 };
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	float threshold = 150.0f, speed = 4.0f;
	AIStateMachine stateMachine;
	Sprite rect;
};