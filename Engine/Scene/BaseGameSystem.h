#pragma once

class BaseGameSystem {
public:
    virtual ~BaseGameSystem() {}
    virtual void OnUpdate() = 0;
    virtual void OnFinalize() = 0;
    virtual bool IsTerminateSystem() const = 0;
};