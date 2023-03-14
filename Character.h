#pragma once
#include <chrono>
#include "Sprite.h"

class Character {
public:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    struct Vertex {
        XMFLOAT2 pos;
        XMFLOAT4 color;
        XMFLOAT2 tex;
    };
    Character(const char* idleFrames, const char* walkingFrames, const char* hitFrames, const char* dashFrames, float nWidth, float nHeight, Sprite* sprite, HWND hwnd, Graphics* graphics)
        : width(nWidth), height(nHeight), spriteLoader(sprite), windowHandle(hwnd), gfx(graphics) {
        float aspectRatio = (float)width / (float)height;
        Vertex OurVertices[] =
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
        bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(OurVertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA srd = { OurVertices, 0, 0 };
        this->gfx->d3dDevice->CreateBuffer(&bd, &srd, this->vertexBuffer.GetAddressOf());
        this->idleFrames = spriteLoader->LoadFromDir(idleFrames, nWidth, nHeight);
        this->walkingFrames = spriteLoader->LoadFromDir(walkingFrames, nWidth, nHeight);
        this->hitFrames = spriteLoader->LoadFromDir(hitFrames, nWidth, nHeight);
        this->dashFrames = spriteLoader->LoadFromDir(dashFrames, nWidth, nHeight);
        dashState = Ready;
    }
    void Update() {
        if (!isHitting) {
            isMoving = false;
            if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
                isMoving = true;
                if (dashState != Dashing && !facingRight) { Flip(); }
                x -= speed;
            }
            if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
                isMoving = true;
                if (dashState != Dashing && facingRight) { Flip(); }
                x += speed;
            }
            if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
                isMoving = true;
                y += speed;
            }
            if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
                isMoving = true;
                y -= speed;
            }
            switch (dashState) {
            case Ready:
                dashFrame = 0;
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                    GetCursorPos(&pt);
                    ScreenToClient(gfx->windowHandle, &pt);
                    float pos_x = 5 * gfx->width * (2.0f * pt.x / gfx->width - 1.0f);
                    float pos_y = -5 * gfx->height * (2.0f * pt.y / gfx->height - 1.0f);
                    angle = atan2(this->y - pos_y, this->x - pos_x);
                    std::cout << pos_x << " " << this->x << std::endl;
                    distance = sqrt(pow(this->x - pos_x, 2) + pow(this->y - pos_y, 2));
                    distance = smoothstep(0.0f, 3.0f, distance);
                    dashState = Dashing;
                    this->facingRight = (pos_x < this->x) ? true : false;
                }
                break;
            case Dashing: {
                isMoving = false;
                isHitting = false;
                dt += 0.15;
                this->x -= cos(angle) * distance * 100.0f;
                this->y -= sin(angle) * distance * 100.0f;
                if (dt >= 8.0) {
                    dashState = CoolDown;
                    dt += 6.0;
                }
                break;
            }
            case CoolDown:
                dt -= 0.15;
                if (dt <= 0) {
                    dashState = Ready;
                }
                dashFrame = 0;
                break;
            }
        }
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && dashState != Dashing && hitEnabled) {
            isHitting = true;
            isMoving = false;
            hitCooldownStart = std::chrono::high_resolution_clock::now();
            hitEnabled = false;
        }
        damageEnabled = (this->dashState == this->Dashing) ? true : false;
    }
    void UpdateTimeless() {
        if (!isHitting) {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - hitCooldownStart).count();
            if (duration > 500) {
                hitEnabled = true;
            }
        }
        frame++;
    }
    void Render() {
        if (isMoving && !isHitting) this->currentFrame = this->walkingFrames[(frame / 15) % 4];
        else if (dashState != Dashing && !isHitting && !isMoving) this->currentFrame = this->idleFrames[(frame / 15) % 4];
        else if (dashState == Dashing) {
            if (dashFrame < 54) {
                this->currentFrame = this->dashFrames[dashFrame / 6];
                dashFrame++;
            }
        }
        else if (isHitting && !isMoving) {
            if (hitFrame < 32) {
                this->currentFrame = this->hitFrames[hitFrame / 8];
                hitFrame++;
            }
            else {
                hitFrame = 0;
                isHitting = false;
                damageEnabled = true;
            }
        }
        //if (this->currentFrame != nullptr) spriteLoader->DrawTexture(this->currentFrame, x, y, this->facingRight);
    }
    ID3D11ShaderResourceView* currentFrame;
    float x = 100, y = 100;
    float width, height;
    bool isHitting = false, damageEnabled = true;
    float health = 1000.0f;
    enum DashStates {
        Ready,
        Dashing,
        CoolDown
    };
    DashStates dashState;
    POINT pt;
    bool facingRight = false;
private:
    double dt = 0;
    float distance, angle;
    float smoothstep(float edge0, float edge1, float x) {
        x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    };
    float clamp(float x, float min, float max) {
        return fmin(fmax(x, min), max);
    };
    HWND windowHandle;
    void Flip() {
        facingRight = !facingRight;
    }
    float speed = 10.0f;
    std::vector<ID3D11ShaderResourceView*> hitFrames;
    std::vector<ID3D11ShaderResourceView*> dashFrames;
    std::vector<ID3D11ShaderResourceView*> walkingFrames;
    std::vector<ID3D11ShaderResourceView*> idleFrames;
    int frame = 0, hitFrame = 0, dashFrame = 0;
    Sprite* spriteLoader;
    Graphics* gfx;
    bool isMoving = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> hitCooldownStart;
    bool hitEnabled = true;
};