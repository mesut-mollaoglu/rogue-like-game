#pragma once

#include "SaveSystem.h"

class Character {
public:
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11Buffer> constantBuffer;
    Character(const char* idleFrames, const char* walkingFrames, const char* hitFrames, const char* dashFrames, float nWidth, float nHeight, Sprite* spriteLoader)
        : width(nWidth), height(nHeight), sprite(spriteLoader) {
        float aspectRatio = (float)width / (float)height;
        Graphics::Vertex vertices[] =
        {
            XMFLOAT2(-0.650 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 1),
            XMFLOAT2(-0.650 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 0),
            XMFLOAT2(0.650 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 0),
            XMFLOAT2(0.650 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 1)
        };
        Graphics::CreateVertexBuffer(vertexBuffer, vertices, ARRAYSIZE(vertices));
        Graphics::CreateIndexBuffer(indexBuffer);
        Graphics::CreateConstantBuffer<Graphics::Constants>(constantBuffer);
        stateMachine.AddState(walking, new Animator(sprite->LoadFromDir(walkingFrames, nWidth, nHeight), 250), "Walking", {'W', 'A', 'S', 'D'});
        stateMachine.AddState(idle, new Animator(sprite->LoadFromDir(idleFrames, nWidth, nHeight), 250), "Idle", {});
        stateMachine.AddState(dash, new Animator(sprite->LoadFromDir(dashFrames, nWidth, nHeight), 50, true), "Dash", {VK_SHIFT}, 2500);
        stateMachine.AddState(attack, new Animator(sprite->LoadFromDir(hitFrames, nWidth, nHeight), 125, true), "Attack", {VK_LBUTTON});
        stateMachine.SetState("Idle");
        SaveSystem::FileInit("Character.txt");
        StringToPosition();
    }
    std::function<void()> walking = [&, this]() {
        states = States::LoadDash;
        if (StateMachine::isKeyPressed('A')) {
            if (!stateMachine.equals("Dash") && !facingRight) { Flip(); }
            position -= {speed, 0};
        }
        if (StateMachine::isKeyPressed('D')) {
            if (!stateMachine.equals("Dash") && facingRight) { Flip(); }
            position += {speed, 0};
        }
        if (StateMachine::isKeyPressed('W')) {
            position += {0, speed};
        }
        if (StateMachine::isKeyPressed('S')) {
            position -= {0, speed};
        }
        if (!SaveSystem::isCurrentFile("Character.txt"))
            SaveSystem::FileInit("Character.txt");
        SaveSystem::DeleteLine(1);
        this->PositionToString();
    };
    std::function<void()> dash = [&, this]() {
        switch (states) {
        case States::LoadDash: {
            Math::float2 mousePos = StateMachine::ToScreenCoord(StateMachine::GetMousePos());
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
        Graphics::SetEyePosition(Math::float3(0, 0, 5));
        stateMachine.UpdateState();
    }
    void Render() {
        Graphics::SetConstantValues<Graphics::Constants>(this->constantBuffer.Get(), { XMFLOAT2{(this->GetPosition().x - Graphics::GetEyeDistance().x) / Structures::Window::GetWidth(),
            (this->GetPosition().y - Graphics::GetEyeDistance().y) / Structures::Window::GetHeight()}, XMFLOAT2{0, 0}, XMFLOAT4{(this->facingRight) ? -1.0f : 1.0f, 0, 0, 0} });
        ID3D11ShaderResourceView* currentFrame = stateMachine.RenderState();
        Graphics::d3dDeviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf()); 
        Graphics::d3dDeviceContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
        Graphics::d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        Graphics::d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &Graphics::stride,
            &Graphics::offset);
        Graphics::d3dDeviceContext->PSSetShaderResources(0, 1, &currentFrame);
        Graphics::d3dDeviceContext->DrawIndexed(6, 0, 0);
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
    void StringToPosition() {
        if (!SaveSystem::isCurrentFile("Character.txt")) return;
        std::string data = SaveSystem::ReadData();
        this->position.toFloat(data);
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
    float health = 100.0f;
    float distance = 0.0f, angle = 0.0f;
    void Flip() {
        facingRight = !facingRight;
    }
    float speed = 10.0f;
    StateMachine stateMachine;
    Sprite* sprite;
};