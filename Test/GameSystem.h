#pragma once

#include "Scene/BaseGameSystem.h"


class GameSystem :
    public BaseGameSystem {
public:
    static GameSystem* GetInstance();

    void OnUpdate() override;
    void OnFinalize() override;
    bool IsTerminateSystem() const override;
    

private:

};