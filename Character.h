#pragma once

#include "Engine/Primitives.h"

class Character {
public:
    Character(std::string idleFrames, std::string walkingFrames, std::string hitFrames, std::string dashFrames) {
        stateMachine.AddState(walking, new Animator(Graphics::LoadFromDir(walkingFrames), 250), "Walking", { 'W', 'A', 'S', 'D' });
        MapKeyBoard();
        stateMachine.AddState(idle, new Animator(Graphics::LoadFromDir(idleFrames), 250), "Idle", {});
        stateMachine.AddState(dash, new Animator(Graphics::LoadFromDir(dashFrames), 50, true), "Dash", { VK_SHIFT }, 2000);
        stateMachine.AddState(attack, new Animator(Graphics::LoadFromDir(hitFrames), 125, true), "Attack", { VK_LBUTTON });
        stateMachine.SetState("Idle");
        rect = Primitives::Sprite();
    }
    void MapKeyBoard() {
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[0], Math::Vec2f(0, 1)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[1], Math::Vec2f(-1, 0)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[2], Math::Vec2f(0, -1)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].mKeys[3], Math::Vec2f(1, 0)));
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
            Math::Vec2f mousePos = BaseStateMachine::ToScreenCoord(BaseStateMachine::GetMousePos());
            angle = Math::GetAngle(position, mousePos);
            distance = position.GetDistance(mousePos);
            distance = Math::smoothstep(0.0f, 3.0f, distance);
            this->facingRight = (mousePos.x < position.x) ? true : false;
            states = States::UpdateDash;
        }
                             break;
        case States::UpdateDash: {
            if (InBounds()) {
                prevPosition = position;
                position -= Math::toVector(angle) * distance * 100.0f;
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
        }
    };
    void Update() {
        stateMachine.UpdateState();
    }
    bool InBounds() {
        return position.x < 4500 && position.x > -4500 && position.y > -2100 && position.y < 3280;
    }
    void Render() {
        rect.SetPosition(GetPosition());
        rect.Draw(stateMachine.RenderState(), facingRight ? Primitives::FlipHorizontal::FlippedHorizontal : Primitives::FlipHorizontal::NormalHorizontal);
    }
    Math::Vec2f GetPosition() {
        return position;
    }
    float GetHealth() {
        return health;
    }
    void SetHealth(float val) {
        health = val;
    }
    void SetPosition(Math::Vec2f pos) {
        this->position = pos;
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
        stateMachine.Clear();
        rect.Free();
    }
    float nDamage = 0.0f;
private:
    Math::Vec2f position, prevPosition;
    std::vector<std::pair<BaseStateMachine::Key, Math::Vec2f>> vKeyMap;
    float health = 100.0f;
    float distance = 0.0f, angle = 0.0f;
    float speed = 10.0f;
    StateMachine stateMachine;
    Primitives::Sprite rect;
};