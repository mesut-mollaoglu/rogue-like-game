#pragma once
#include "Engine/Primitives.h"
#include <stack>

class Entity {
public:
	typedef std::unique_ptr<Entity>(*FactoryFunction)();
	virtual void Update(Vec2f pos) = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;
	bool InBounds() {
		return position.x < 4500 && position.x > -4500 && position.y > -2100 && position.y < 2100;
	}
	bool InBounds(Vec2f position) {
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
	bool IsCurrentState(std::string state) {
		return stateMachine.IsCurrentState(state);
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
	Enemy() {
		nDamage = 0.f;
		health = 10.f;
		rect = Sprite();
		stateMachine.AddState(Follow, new Animator(Graphics::LoadFromDir("Assets\\Enemy\\moveAnim"), 250), "Follow");
		stateMachine.AddState(Attack, new Animator(Graphics::LoadFromDir("Assets\\Enemy\\attackAnim"), 75, true), "Attack");
		stateMachine.AddState(Dead, new Animator(Graphics::LoadFromDir("Assets\\Enemy\\deadAnim"), 250, true), "Dead");
		stateMachine.AddState(Idle, new Animator(Graphics::LoadFromDir("Assets\\Enemy\\idleAnim"), 250), "Idle");
	}
	static inline std::unique_ptr<Entity> Create() { return std::make_unique<Enemy>(); }
	std::function<void()> Dead = [&, this]() {
		if (AnimEnd()) bDead = true;
	};
	std::function<void()> Follow = [&, this]() {
		if (!bEnteredBounds && InBounds()) bEnteredBounds = true;
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
			if (AnimEnd() && characterPosition.GetDistance(position) < 550.f)
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
		characterPosition = pos;
		if (health <= 0.f) SetState("Dead");
		float distance = characterPosition.GetDistance(position);
		if (!bEnteredBounds || (distance < 7000.0f && distance > 550.0f)) SetState("Follow");
		else if (distance >= 7000.0f) SetState("Idle");
		else if (distance <= 550.0f) SetState("Attack");
		elapsedTime += 0.01f;
		if (!IsCurrentState("Attack")) {
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
private:
	enum class AttackStates {
		DamagePlayer,
		Cooldown,
	}attackStates = AttackStates::DamagePlayer;
	float elapsedTime = 0.0f;
	float smoothSin(float time, float frequency, float amplitude) {
		return amplitude * sin(2 * PI * frequency * time);
	};
	float m_time = 0.0;
	float m_deltaTime = 1 / 100;
	float threshold = 150.0f, speed = 4.0f;
	bool bEnteredBounds = false;
};

struct EnergyBall {
	timePoint tp;
	Vec2f position;
	float angle, distance;
	Sprite rect;
	EnergyBall() = default;
	EnergyBall(Vec2f pos) {
		position = pos;
		rect = Sprite();
		rect.SetTexture(Graphics::LoadTexture("Assets\\RangedEnemy\\energyBall.png"), 0.4f);
		tp = Clock::now();
	}
	void Update(Vec2f pos) {
		if (distance > 4000.f) return;
		angle = GetAngle(position, pos);
		position -= toVector(angle) * 50.f;
		distance += (toVector(angle) * 50.f).GetLength();
	}
	void Render() {
		rect.SetPosition(position);
		rect.Draw();
	}
	void Destroy() {
		rect.Free();
	}
};

class RangedEnemy : public Entity {
public:
	RangedEnemy() {
		nDamage = 0.f;
		health = 30.f;
		rect = Sprite();
		stateMachine.AddState(Spawn, new Animator(Graphics::LoadFromDir("Assets\\RangedEnemy\\spawnAnim"), 200, true), "Spawn");
		stateMachine.AddState(Attack, new Animator(Graphics::LoadFromDir("Assets\\RangedEnemy\\idleAnim"), 250, true), "Attack", 2000.f);
		stateMachine.AddState(Dead, new Animator(Graphics::LoadFromDir("Assets\\RangedEnemy\\deadAnim"), 250, true), "Dead");
		stateMachine.AddState(Idle, new Animator(Graphics::LoadFromDir("Assets\\RangedEnemy\\idleAnim"), 250), "Idle");
		energyBall = EnergyBall(GetPosition());
	}
	static inline std::unique_ptr<Entity> Create() { return std::make_unique<RangedEnemy>(); }
	std::function<void()> Spawn = [&, this]() {
		SetState("Idle");
		nDamage = 0.f;
	};
	std::function<void()> Idle = [&, this]() {
		nDamage = 0.f; 
		energyBall.distance = 0.f;
		energyBall.position = GetPosition();
		attackStates = AttackStates::DamagePlayer;
		SetState("Attack");
	};
	std::function<void()> Attack = [&, this]() {
		energyBall.Update(characterPosition); 
		if (energyBall.position.GetDistance(characterPosition) < 800.f) {
			if (attackStates == AttackStates::DamagePlayer) nDamage = 25.f;
			else if (attackStates == AttackStates::Cooldown) nDamage = 0.f;
			attackStates = AttackStates::Cooldown;
			SetState("Idle");
		}
		if (!InBounds(energyBall.position) || energyBall.distance >= 4000.f) {
			SetState("Idle");
		}
	};
	std::function<void()> Dead = [&, this]() {
		if (AnimEnd()) bDead = true;
	};
	void Update(Vec2f pos) override {
		characterPosition = pos;
		if (GetHealth() <= 0.f) SetState("Dead");
		stateMachine.UpdateState();
	}
	void Render() override {
		rect.SetPosition(GetPosition());
		rect.SetTexture(stateMachine.RenderState());
		rect.Draw();
		if (IsCurrentState("Attack")) energyBall.Render();
	}
	void Destroy() override {
		energyBall.Destroy();
		stateMachine.Clear();
		rect.Free();
		std::destroy_at(std::addressof(Spawn)); 
		std::destroy_at(std::addressof(Idle));
		std::destroy_at(std::addressof(Attack));
		std::destroy_at(std::addressof(Dead));
	}
private:
	enum class AttackStates {
		DamagePlayer,
		Cooldown,
	}attackStates = AttackStates::DamagePlayer;
	EnergyBall energyBall;
};