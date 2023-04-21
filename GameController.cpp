#include "GameController.h"
#include "Enemy.h"
#include "Character.h"

void GameController::Load(Sprite* sprite, HWND hwnd) {
    spriteLoader = sprite;
    enemies.push_back(std::make_unique<Enemy>("EnemyAttackFrames", "EnemyMovingFrames", "EnemyIdleFrames", "EnemyDeadFrames", 152, 148, spriteLoader, gfx));
    character = std::make_unique<Character>("IdleAnimFrames", "WalkingAnimFrames", "HitAnimFrames", "DashAnimFrames", 192, 138, spriteLoader, gfx);
    Menu = std::make_unique<MainMenu>(spriteLoader, gfx, hwnd);
    states = mainMenu;
}

void GameController::Unload() {
    character.reset();
}

void GameController::Update(HWND windowHandle, MSG msg) {
    switch (states) {
    case mainMenu:{
        Menu->Update();
        if (Menu->bGameRunning) states = gameRunning;
    }
    case gameRunning: {
        if (!bInit)
        {
            start = now = Clock::now();
            bInit = true;
        }
        now = Clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (duration.count() > 150) {
            for (int i = 0; i < enemies.size(); i++) {
                enemies[i]->Update(character->GetPosition());
                if (spriteLoader->CheckCollision(character.get(), enemies[i].get())) {
                    if (character.get()->GetStateMachine().equals("Dash")) {
                        enemies[i]->health = 0;
                    }
                    if (character.get()->GetStateMachine().equals("Attack")) {
                        enemies[i]->health -= 10;
                    }
                    else if (enemies[i]->attacking && enemies[i].get()->damageEnabled){
                        character->SetHealth(character->GetHealth() - 10);
                        enemies[i].get()->damageEnabled = false;
                    }
                }
                if (enemies[i].get()->isDead) enemies.erase(enemies.begin() + i);  
            }
            character->Update();
        }
        }
        if (StateMachine::isKeyPressed(VK_ESCAPE)) {
            states = mainMenu;
            Menu->bGameRunning = false;
        break;
    }
    }
}

void GameController::Render() {
    switch (states) {
    case mainMenu: {
        Menu->Render();
        break;
    }
    case gameRunning: {
        D3D11_MAPPED_SUBRESOURCE projectionSubresource;
        gfx->d3dDeviceContext->Map(gfx->projectionBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &projectionSubresource);
        Graphics::ProjectionBuffer* projectionMatrix = (Graphics::ProjectionBuffer*)(projectionSubresource.pData);
        projectionMatrix->proj = Structures::Camera::projMatrix;
        projectionMatrix->view = Structures::Camera::viewMatrix;
        projectionMatrix->world = Structures::Camera::worldMatrix;
        gfx->d3dDeviceContext->Unmap(gfx->projectionBuffer.Get(), 0);
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        gfx->d3dDeviceContext->Map(gfx->constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        Graphics::Constants* constants = (Graphics::Constants*)(mappedSubresource.pData);
        constants->pos = { character->GetPosition().x / Structures::Window::GetWidth(), character->GetPosition().y / Structures::Window::GetHeight() };
        constants->horizontalScale = { (character->facingRight) ? -1.0f : 1.0f, 0, 0, 0 };
        gfx->d3dDeviceContext->Unmap(gfx->constantBuffer.Get(), 0);
        D3D11_MAPPED_SUBRESOURCE subresource;
        gfx->d3dDeviceContext->Map(gfx->enemyConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
        Graphics::Constants* enemyConstants = (Graphics::Constants*)(subresource.pData);
        enemyConstants->pos = { enemies[0]->GetPosition().x / Structures::Window::GetWidth(), enemies[0]->GetPosition().y / Structures::Window::GetHeight() };
        enemyConstants->horizontalScale = { (enemies[0]->facingRight) ? -1.0f : 1.0f, 
            (enemies[0]->isDead) ? 1.0f : 0.0f, 0, 0 };
        gfx->d3dDeviceContext->Unmap(gfx->enemyConstantBuffer.Get(), 0);
        gfx->Clear(0.5f, 0.5f, 0.5f, 1.0f);
        gfx->Begin();
        character->Render();
        this->gfx->d3dDeviceContext->PSSetShader(this->gfx->pixelShader.Get(), nullptr, 0);
        for (int i = 0; i < enemies.size(); i++){
            if (!enemies[i]->isDead){
                enemies[i].get()->Render();
                this->gfx->d3dDeviceContext->PSSetConstantBuffers(0, 1, gfx->enemyConstantBuffer.GetAddressOf());
                this->gfx->d3dDeviceContext->VSSetConstantBuffers(0, 1, gfx->enemyConstantBuffer.GetAddressOf());
                this->gfx->d3dDeviceContext->IASetIndexBuffer(enemies[i]->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                this->gfx->d3dDeviceContext->IASetVertexBuffers(0, 1, enemies[i]->vertexBuffer.GetAddressOf(), &this->gfx->stride,
                    &this->gfx->offset);
                this->gfx->d3dDeviceContext->PSSetShaderResources(0, 1, &enemies[i]->currentFrame);
                this->gfx->d3dDeviceContext->DrawIndexed(6, 0, 0);
            }
        }
        gfx->End();
        break;
    }
    }
}