#pragma once
#include "Engine/Primitives.h"

class Entity {
public:
	virtual void Update(Vec2f pos) = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;
	bool InBounds() {
		return position.x < 4500 && position.x > -4500 && position.y > -2100 && position.y < 2100;
	}
	Vec2f GetPosition() {
		return position;
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
	float GetHealth() {
		return health;
	}
	const char* GetState() {
		return stateMachine.GetState();
	}
	float health;
	bool facingRight = false;
	bool bDead = false;
	Vec2f characterPosition;
	Vec2f position;
	AIStateMachine stateMachine;
	Sprite rect;
	float nDamage;
};

class Enemy : public Entity {
public:
	Enemy(std::string attackDir, std::string moveDir, std::string idleDir, std::string deadDir) {
		nDamage = 0.f;
		health = 10.f;
		rect = Sprite();
		stateMachine.AddState(Follow, new Animator(Graphics::LoadFromDir(moveDir), 250), "Follow");
		stateMachine.AddState(Idle, new Animator(Graphics::LoadFromDir(idleDir), 250), "Idle");
		stateMachine.AddState(Attack, new Animator(Graphics::LoadFromDir(attackDir), 75, true), "Attack");
		stateMachine.AddState(Dead, new Animator(Graphics::LoadFromDir(deadDir), 250, true), "Dead");
	}
	std::function<void()> Dead = [&, this]() {};
	std::function<void()> Follow = [&, this]() {
		attackStates = AttackStates::DamagePlayer;
		float angle = GetAngle(position, characterPosition);
		float t = elapsedTime * speed;
		t = Smoothstep(0.0f, 20.0f, t);
		position -= toVector(angle) * t * 30.0f;
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
			if (AnimEnd() && characterPosition.GetDistance(position) < 1000.f)
				attackStates = AttackStates::DamagePlayer;
		}
								   break;
		}};
	std::function<void()> Idle = [&, this]() {attackStates = AttackStates::DamagePlayer; };
	void Render() override {
		rect.SetPosition(GetPosition());
		rect.SetTexture(stateMachine.RenderState());
		rect.Draw(facingRight ? FlipHorizontal::FlippedHorizontal : FlipHorizontal::NormalHorizontal);
	}
	void Update(Vec2f pos) override {
		if (InBounds()) bEnteredBounds = true;
		characterPosition = pos;
		if (health <= 0.f) SetState("Dead");
		if (isState("Dead") && AnimEnd()) bDead = true;
		float distance = characterPosition.GetDistance(position);
		if (!bEnteredBounds || (distance < 7000.0f && distance > 550.0f)) SetState("Follow");
		else if (distance >= 7000.0f) SetState("Idle");
		else if (distance <= 550.0f) SetState("Attack");
		elapsedTime += 0.01f;
		if (!isState("Attack")) {
			nDamage = 0.f;
			position.y += smoothSin(elapsedTime, 1.5f, 6.0f);
			m_time += m_deltaTime;
		}
		facingRight = (position.x > characterPosition.x) ? true : false;
		stateMachine.UpdateState();
	}
	void Destroy() override {
		stateMachine.Clear();
		rect.Free();
		std::destroy_at(std::addressof(Follow));
		std::destroy_at(std::addressof(Dead));
		std::destroy_at(std::addressof(Attack));
		std::destroy_at(std::addressof(Idle));
	}
	virtual ~Enemy() {}
	enum class AttackStates {
		DamagePlayer,
		Cooldown,
	}attackStates = AttackStates::DamagePlayer;
private:
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	float threshold = 150.0f, speed = 4.0f;
	bool bEnteredBounds = false;
};