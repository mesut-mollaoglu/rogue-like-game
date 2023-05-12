#include "GameController.h"
#include "Enemy.h"
#include "Character.h"

std::string SaveSystem::fileName;
std::fstream SaveSystem::currentFile;

void GameController::Load() {
    enemies.push_back(std::make_unique<Enemy>("EnemyAttackFrames", "EnemyMovingFrames", "EnemyIdleFrames", "EnemyDeadFrames", 152, 148));
    character = std::make_unique<Character>("IdleAnimFrames", "WalkingAnimFrames", "HitAnimFrames", "DashAnimFrames", 192, 138);
    Menu = std::make_unique<MainMenu>();
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
                if (Sprite::CheckCollision(character.get(), enemies[i].get())) {
                    if (character.get()->GetStateMachine().equals("Dash")) {
                        enemies[i]->health = 0;
                    }
                    if (character.get()->GetStateMachine().equals("Attack")) {
                        enemies[i]->health -= 10;
                    }
                }
            }
            character->Update();
        }
        }
        if (BaseStateMachine::isKeyPressed(VK_ESCAPE)) {
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
        using Structures::Camera;
        Graphics::SetConstantValues<Graphics::ProjectionBuffer>(Graphics::projectionBuffer.Get(), {
            Camera::projMatrix, Camera::worldMatrix, Camera::viewMatrix});
        Graphics::Clear(0.5f, 0.5f, 0.5f, 1.0f);
        Graphics::Begin();
        character->Render();
        Graphics::d3dDeviceContext->PSSetShader(Graphics::pixelShader.Get(), nullptr, 0);
        for (int i = 0; i < enemies.size(); i++){
            enemies[i].get()->Render();
        }
        Graphics::End();
        break;
    }
    }
}