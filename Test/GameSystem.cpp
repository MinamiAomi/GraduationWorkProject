#include "GameSystem.h"

GameSystem* GameSystem::GetInstance() {
    static GameSystem instance;
    return &instance;
}

void GameSystem::OnUpdate() {

}

void GameSystem::OnFinalize() {

}

bool GameSystem::IsTerminateSystem() const {
    return false;
}