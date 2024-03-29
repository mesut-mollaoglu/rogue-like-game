#include "GameController.h"

void GameController::Load() {
    vLevels[nLevelIndex]->Load();
}

void GameController::Unload() {
    vLevels[nLevelIndex]->UnLoad();
}

void GameController::Update() {
    if (vLevels[nLevelIndex]->nLevel == Level::ManageLevel::CurrentLevel) {

    }
    else if (vLevels[nLevelIndex]->nLevel == Level::ManageLevel::PrevLevel) {
        vLevels[nLevelIndex]->nLevel = Level::ManageLevel::CurrentLevel;
        vLevels[nLevelIndex]->UnLoad();
        nLevelIndex = (nLevelIndex - 1) % vLevels.size();
        vLevels[nLevelIndex]->Load();
    }
    else if (vLevels[nLevelIndex]->nLevel == Level::ManageLevel::NextLevel) {
        vLevels[nLevelIndex]->nLevel = Level::ManageLevel::CurrentLevel;
        vLevels[nLevelIndex]->UnLoad();
        nLevelIndex = (nLevelIndex + 1) % vLevels.size();
        vLevels[nLevelIndex]->Load();
    }
    else if (vLevels[nLevelIndex]->nLevel == Level::ManageLevel::GotoLevel) {
        vLevels[nLevelIndex]->nLevel = Level::ManageLevel::CurrentLevel;
        vLevels[nLevelIndex]->UnLoad();
        int index = vLevels[nLevelIndex]->nIndex;
        vLevels[nLevelIndex]->nIndex = 0;
        nLevelIndex = index % vLevels.size();
        vLevels[nLevelIndex]->Load();
    }
    if (!bInit)
    {
        start = now = Clock::now();
        bInit = true;
    }
    now = Clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    if (duration.count() > vLevels[nLevelIndex]->ticks) {
        vLevels[nLevelIndex]->FixedUpdate();
    }
    vLevels[nLevelIndex]->Update();
}

void GameController::Render() {
    vLevels[nLevelIndex]->Render();
}