#pragma once

#include "Engine/Primitives.h"

class Character {
public:
    Character(std::string idleFrames, std::string walkingFrames, std::string hitFrames, std::string dashFrames, float maxHealth = 300) {
        mHealth = HealthBar(maxHealth, maxHealth, 0);
        mHealth.SetPosition({ ToScreenCoord({120, 30})});
        mHealth.SetTexture(Graphics::LoadTexture("healthbar.png"), .25f);
        SetHealth(maxHealth);
        stateMachine.AddState(walking, new Animator(Graphics::LoadFromDir(walkingFrames), 250), "Walking", { 'W', 'A', 'S', 'D' });
        MapKeyBoard();
        stateMachine.AddState(idle, new Animator(Graphics::LoadFromDir(idleFrames), 250), "Idle", {});
        stateMachine.AddState(dash, new Animator(Graphics::LoadFromDir(dashFrames), 50, true), "Dash", { VK_SHIFT }, 2000);
        stateMachine.AddState(attack, new Animator(Graphics::LoadFromDir(hitFrames), 125, true), "Attack", { VK_LBUTTON });
        stateMachine.SetState("Idle");
        rect = Sprite();
    }
    void MapKeyBoard() {
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[0], Vec2f(0, 1)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[1], Vec2f(-1, 0)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[2], Vec2f(0, -1)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[3], Vec2f(1, 0)));
    }
    std::function<void()> walking = [&, this]() {
        states = States::LoadDash;
        attackStates = AttackStates::DamageEnemy;
        if (!InBounds()) {
            position = prevPosition;
            stateMachine.SetState("Idle");
        }
        for (auto& p : vKeyMap)
            if (p.first.isPressed() && InBounds()) {
                prevPosition = position;
                position += p.second * speed;
                if (p.second.x == -1 && !facingRight)
                    Flip();
                else if (p.second.x == 1 && facingRight)
                    Flip();
            }
    };
    std::function<void()> dash = [&, this]() {
        attackStates = AttackStates::DamageEnemy;
        switch (states) {
        case States::LoadDash: {
            Vec2f mousePos = ToScreenCoord(GetMousePos());
            angle = GetAngle(position, mousePos);
            distance = position.GetDistance(mousePos);
            distance = Smoothstep(0.0f, 3.0f, distance);
            this->facingRight = (mousePos.x < position.x) ? true : false;
            states = States::UpdateDash;
        }
                             break;
        case States::UpdateDash: {
            if (InBounds()) {
                prevPosition = position;
                position -= toVector(angle) * distance * 100.0f;
            }
            else
                position = prevPosition;
        }
                               break;
        }
    };
    std::function<void()> idle = [&, this]() {
        states = States::LoadDash;
        attackStates = AttackStates::DamageEnemy;
    };
    std::function<void()> attack = [&, this]() {
        states = States::LoadDash;
        switch (attackStates) {
        case AttackStates::DamageEnemy: {
            nDamage = 10.f;
            attackStates = AttackStates::Cooldown; 
        }
                                      break;
        case AttackStates::Cooldown: {
            nDamage = 0.f;
        }
                                   break;
        }
    };
    void Update() {
        stateMachine.UpdateState();
    }
    bool InBounds() {
        return position.x < 4500 && position.x > -4500 && position.y > -2100 && position.y < 2100;
    }
    void Render() {
        rect.SetPosition(GetPosition());
        rect.SetTexture(stateMachine.RenderState());
        rect.Draw(facingRight ? FlipHorizontal::FlippedHorizontal : FlipHorizontal::NormalHorizontal);
    }
    void RenderHealth() {
        mHealth.Draw();
    }
    Vec2f GetPosition() {
        return position;
    }
    float GetHealth() {
        return health;
    }
    void SetHealth(float val) {
        health = val;
        mHealth.SetHealth(health);
    }
    void SetPosition(Vec2f pos) {
        position = pos;
    }
    enum class States {
        LoadDash,
        UpdateDash
    };
    States states = States::LoadDash;
    enum class AttackStates {
        DamageEnemy,
        Cooldown,
    }attackStates = AttackStates::DamageEnemy;
    bool facingRight = true;
    StateMachine GetStateMachine() {
        return stateMachine;
    }
    void Flip() {
        facingRight = !facingRight;
    }
    bool isState(std::string state) {
        return stateMachine.equals(state);
    }
    void Destroy() {
        for (auto& key : vKeyMap)
            key.first.keys.clear();
        stateMachine.Clear();
        rect.Free();
        mHealth.Free();
        std::destroy_at(std::addressof(walking));
        std::destroy_at(std::addressof(dash));
        std::destroy_at(std::addressof(idle));
        std::destroy_at(std::addressof(attack));
    }
    const char* GetState() {
        return stateMachine.GetState();
    }
    void SetState(std::string state) {
        stateMachine.SetState(state);
    }
    void SetSpeed(float fSpeed) {
        speed = fSpeed;
    }
    float GetSpeed() {
        return speed;
    }
    float nDamage = 0.0f;
private:
    Vec2f position, prevPosition;
    std::vector<std::pair<BaseStateMachine::Key, Vec2f>> vKeyMap;
    float health = 300.f;
    float distance = 0.0f, angle = 0.0f;
    float speed = 10.0f;
    StateMachine stateMachine;
    Sprite rect;
    HealthBar mHealth;
};