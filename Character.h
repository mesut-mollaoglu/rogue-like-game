#pragma once

#include "Primitives.h"
#include <set>

class Character {
public:
    Character(const char* idleFrames, const char* walkingFrames, const char* hitFrames, const char* dashFrames, float nWidth, float nHeight)
        : width(nWidth), height(nHeight) {
        stateMachine.AddState(walking, new Animator(Sprite::LoadFromDir(walkingFrames, nWidth, nHeight), 250), "Walking", { 'W', 'A', 'S', 'D' });
        MapKeyBoard();
        stateMachine.AddState(idle, new Animator(Sprite::LoadFromDir(idleFrames, nWidth, nHeight), 250), "Idle", {});
        stateMachine.AddState(dash, new Animator(Sprite::LoadFromDir(dashFrames, nWidth, nHeight), 50, true), "Dash", { VK_SHIFT }, 2000);
        stateMachine.AddState(attack, new Animator(Sprite::LoadFromDir(hitFrames, nWidth, nHeight), 125, true), "Attack", { VK_LBUTTON });
        stateMachine.SetState("Idle");
        StringToPosition();
        rect = new PrimitiveShapes::TexturedRect();
    }
    void MapKeyBoard() {
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].activationKeys[0], Math::float2(0, 1)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].activationKeys[1], Math::float2(-1, 0)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].activationKeys[2], Math::float2(0, -1)));
        vKeyMap.push_back(std::make_pair(stateMachine.states[0].activationKeys[3], Math::float2(1, 0)));
    }
    std::function<void()> walking = [&, this]() {
        states = States::LoadDash;
        for (auto& p : vKeyMap)
            if (p.first.isPressed()) {
                position += p.second * speed;
                if (p.second.x == -1 && !facingRight)
                    Flip();
                else if (p.second.x == 1 && facingRight)
                    Flip();
            }
    };
    std::function<void()> dash = [&, this]() {
        switch (states) {
        case States::LoadDash: {
            Math::float2 mousePos = BaseStateMachine::ToScreenCoord(BaseStateMachine::GetMousePos());
            angle = Math::GetAngle(position, mousePos);
            distance = position.GetDistance(mousePos);
            distance = Math::smoothstep(0.0f, 3.0f, distance);
            this->facingRight = (mousePos.x < position.x) ? true : false;
            states = States::UpdateDash;
        }
        break;
        case States::UpdateDash: {
            position -= Math::toVector(angle) * distance * 100.0f;
        }
        break;
        }
    };
    std::function<void()> idle = [&, this]() {
        states = States::LoadDash;
    };
    std::function<void()> attack = [&, this]() {
        states = States::LoadDash;
    };
    void Update() {
        if (BaseStateMachine::isKeyPressed('Q'))
        {
            SaveCharacterData();
        }
        stateMachine.UpdateState();
    }
    void Render() {
        rect->Draw(stateMachine.RenderState(), GetPosition(), facingRight ? PrimitiveShapes::FlipHorizontal::FlippedHorizontal : PrimitiveShapes::FlipHorizontal::NormalHorizontal);
    }
    Math::float2 GetPosition() {
        return position;
    }
    float GetHealth() {
        return health;
    }
    void SetHealth(float val) {
        health = val;
    }
    void SetPosition(Math::float2 pos) {
        this->position = pos;
    }
    void SaveCharacterData() {
        if (!SaveSystem::isCurrentFile("Character.txt"))
            SaveSystem::FileInit("Character.txt");
        SaveSystem::DeleteLine(1);
        this->PositionToString();
    }
    void StringToPosition() {
        if (!SaveSystem::isCurrentFile("Character.txt")) return;
        std::string data = SaveSystem::ReadLine();
        this->position = Math::float2::toVector(data);
    }
    void PositionToString() {
        SaveSystem::WriteData(this->position.toString(), true);
    }
    float width, height;
    enum class States {
        LoadDash,
        UpdateDash
    };
    States states = States::LoadDash;
    bool facingRight = false;
    StateMachine GetStateMachine() {
        return stateMachine;
    }
private:
    Math::float2 position;
    std::vector<std::pair<BaseStateMachine::Key, Math::float2>> vKeyMap;
    float health = 100.0f;
    float distance = 0.0f, angle = 0.0f;
    void Flip() {
        facingRight = !facingRight;
    }
    float speed = 10.0f;
    StateMachine stateMachine;
    PrimitiveShapes::TexturedRect* rect;
};