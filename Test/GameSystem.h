#pragma once

#include "Scene/BaseGameSystem.h"


class GameSystem :
    public BaseGameSystem {
public:
    static GameSystem* GetInstance();

    void OnUpdate() override;
    void OnFinalize() override;
    bool IsTerminateSystem() const override;
    
    void Quit();

private:
    GameSystem() = default;
    ~GameSystem() = default;
    GameSystem(const GameSystem&) = delete;
    GameSystem& operator=(const GameSystem&) = delete;

    bool isTerminate_ = false;

};