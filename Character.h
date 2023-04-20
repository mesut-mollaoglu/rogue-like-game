#pragma once

#include "StateMachine.h"

class Character {
public:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Character(const char* idleFrames, const char* walkingFrames, const char* hitFrames, const char* dashFrames, float nWidth, float nHeight, Sprite* spriteLoader, Graphics* graphics)
        : width(nWidth), height(nHeight), gfx(graphics), sprite(spriteLoader) {
        float aspectRatio = (float)width / (float)height;
        Graphics::Vertex OurVertices[] =
        {
            XMFLOAT2(-0.650 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 1),
            XMFLOAT2(-0.650 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0, 0),
            XMFLOAT2(0.650 * aspectRatio, 0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 0),
            XMFLOAT2(0.650 * aspectRatio, -0.5), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1, 1)
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
        bd.ByteWidth = sizeof(Graphics::Vertex) * ARRAYSIZE(OurVertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA srd = { OurVertices, 0, 0 };
        this->gfx->d3dDevice->CreateBuffer(&bd, &srd, this->vertexBuffer.GetAddressOf());
        stateMachine.AddState(walking, new Animator(sprite->LoadFromDir(walkingFrames, nWidth, nHeight), 250), "Walking", {'W', 'A', 'S', 'D'});
        stateMachine.AddState(idle, new Animator(sprite->LoadFromDir(idleFrames, nWidth, nHeight), 250), "Idle", {});
        stateMachine.AddState(dash, new Animator(sprite->LoadFromDir(dashFrames, nWidth, nHeight), 50, true), "Dash", {VK_SHIFT}, 2500);
        stateMachine.AddState(attack, new Animator(sprite->LoadFromDir(hitFrames, nWidth, nHeight), 125, true), "Attack", {VK_LBUTTON});
        stateMachine.SetState("Idle");
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
        Math::float3 distance = Graphics::GetEyeDistance();
        if (StateMachine::mouseWheel == StateMachine::MouseWheel::WHEEL_UP) {
            if (distance.z > 2.5f) Graphics::SetEyePosition(Math::float3(distance.xy(), distance.z - 0.1f));
        }
        else if (StateMachine::mouseWheel == StateMachine::MouseWheel::WHEEL_DOWN) {
            if (distance.z < 7.5f) Graphics::SetEyePosition(Math::float3(distance.xy(), distance.z + 0.1f));
        }
        stateMachine.UpdateState();
    }
    void Render() {
        ID3D11ShaderResourceView* currentFrame = stateMachine.RenderState();
        this->gfx->d3dDeviceContext->VSSetConstantBuffers(0, 1, gfx->constantBuffer.GetAddressOf());
        this->gfx->d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        this->gfx->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &gfx->stride,
            &gfx->offset);
        this->gfx->d3dDeviceContext->PSSetShaderResources(0, 1, &currentFrame);
        this->gfx->d3dDeviceContext->DrawIndexed(6, 0, 0);
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
    Math::float2 position = { -2000, 100 };
    float health = 100.0f;
    float distance = 0.0f, angle = 0.0f;
    void Flip() {
        facingRight = !facingRight;
    }
    float speed = 10.0f;
    StateMachine stateMachine;
    Graphics* gfx;
    Sprite* sprite;
};