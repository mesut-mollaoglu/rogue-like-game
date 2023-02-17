#include "GameController.h"
#include "Enemy.h"
#include "Character.h"

void GameController::Load(Sprite* sprite, HWND hwnd) {
    spriteLoader = sprite;
    enemies.push_back(std::make_unique<Enemy>("EnemyAttackFrames", "EnemyMovingFrames", "EnemyIdleFrames", "EnemyDeadFrames", 152, 148, 600, 600, spriteLoader, gfx));
    character = std::make_unique<Character>("IdleAnimFrames", "WalkingAnimFrames", "HitAnimFrames", "DashAnimFrames", 192, 138, spriteLoader, hwnd, gfx);
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
                enemies[i]->Update(character->x, character->y);
                if (spriteLoader->CheckCollision(character.get(), enemies[i].get())) {
                    if (character->damageEnabled) {
                        if(character->isHitting)
                            enemies[i]->health -= 10;
                        if (character->dashState == character->Dashing)
                            enemies[i]->health = 0;
                        character->damageEnabled = false;
                    }
                    else if (enemies[i]->attacking && enemies[i].get()->damageEnabled){
                        character->health -= 10;
                        enemies[i].get()->damageEnabled = false;
                    }
                }
                if (enemies[i].get()->isDead) enemies.erase(enemies.begin() + i);  
            }
            character->Update();
        }
        character->UpdateTimeless();
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
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
        ProjectionBuffer* projectionMatrix = (ProjectionBuffer*)(projectionSubresource.pData);
        projectionMatrix->proj = gfx->projMatrix;
        projectionMatrix->view = gfx->viewMatrix;
        projectionMatrix->world = gfx->worldMatrix;
        gfx->d3dDeviceContext->Unmap(gfx->projectionBuffer.Get(), 0);
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        gfx->d3dDeviceContext->Map(gfx->constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        Constants* constants = (Constants*)(mappedSubresource.pData);
        constants->pos = { character->x / gfx->width, character->y / gfx->height };
        gfx->d3dDeviceContext->Unmap(gfx->constantBuffer.Get(), 0);
        D3D11_MAPPED_SUBRESOURCE subresource;
        gfx->d3dDeviceContext->Map(gfx->enemyConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
        Constants* enemyConstants = (Constants*)(subresource.pData);
        enemyConstants->pos = { enemies[0]->x / gfx->width, enemies[0]->y / gfx->height };
        gfx->d3dDeviceContext->Unmap(gfx->enemyConstantBuffer.Get(), 0);
        const float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
        this->gfx->d3dDeviceContext->ClearRenderTargetView(this->gfx->renderTargetView.Get(),
            clearColor);
        //spriteLoader->spriteBatch->Begin(SpriteSortMode_BackToFront, nullptr, nullptr, this->gfx->depthStencilState.Get(), this->gfx->rasterizerState.Get(), nullptr, XMMatrixIdentity());
        //spriteLoader->spriteBatch->End();
        character->Render();
        this->gfx->d3dDeviceContext->OMSetDepthStencilState(this->gfx->depthStencilState.Get(), 1);
        this->gfx->d3dDeviceContext->RSSetState(this->gfx->rasterizerState.Get());
        this->gfx->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        this->gfx->d3dDeviceContext->VSSetShader(this->gfx->vertexShader.Get(), nullptr, 0);
        this->gfx->d3dDeviceContext->PSSetShader(this->gfx->pixelShader.Get(), nullptr, 0);
        this->gfx->d3dDeviceContext->VSSetConstantBuffers(0, 1, gfx->constantBuffer.GetAddressOf());
        this->gfx->d3dDeviceContext->VSSetConstantBuffers(1, 1, gfx->projectionBuffer.GetAddressOf());
        this->gfx->d3dDeviceContext->PSSetConstantBuffers(0, 1, gfx->constantBuffer.GetAddressOf());
        this->gfx->d3dDeviceContext->IASetInputLayout(this->gfx->inputLayout.Get());
        this->gfx->d3dDeviceContext->PSSetSamplers(0, 1, this->gfx->samplerState.GetAddressOf());
        this->gfx->d3dDeviceContext->IASetIndexBuffer(character->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        this->gfx->d3dDeviceContext->IASetVertexBuffers(0, 1, character->vertexBuffer.GetAddressOf(), &this->gfx->stride,
            &this->gfx->offset);
        this->gfx->d3dDeviceContext->PSSetShaderResources(0, 1, &character->currentFrame);
        this->gfx->d3dDeviceContext->DrawIndexed(6, 0, 0);
        for (int i = 0; i < enemies.size(); i++){
            if (!enemies[i]->isDead){
                enemies[i].get()->Render();
                this->gfx->d3dDeviceContext->PSSetConstantBuffers(0, 1, gfx->enemyConstantBuffer.GetAddressOf());
                this->gfx->d3dDeviceContext->VSSetConstantBuffers(0, 1, gfx->enemyConstantBuffer.GetAddressOf());
                this->gfx->d3dDeviceContext->VSSetConstantBuffers(1, 1, gfx->projectionBuffer.GetAddressOf());
                this->gfx->d3dDeviceContext->IASetIndexBuffer(enemies[0]->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                this->gfx->d3dDeviceContext->IASetVertexBuffers(0, 1, enemies[0]->vertexBuffer.GetAddressOf(), &this->gfx->stride,
                    &this->gfx->offset);
                this->gfx->d3dDeviceContext->PSSetShaderResources(0, 1, &enemies[0]->currentFrame);
                this->gfx->d3dDeviceContext->DrawIndexed(6, 0, 0);
            }
        }
        this->gfx->swapChain->Present(1, NULL);
        break;
    }
    }
}