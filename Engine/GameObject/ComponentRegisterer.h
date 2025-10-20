#pragma once

#include <vector>
#include <string>

#include "GameObject.h"
#include "Component.h"

class ComponentRegisterer {
public:
    ComponentRegisterer(const std::vector<std::string>&& components) : registeredComponentNames_(components) {}
    virtual ~ComponentRegisterer() {}
    virtual std::shared_ptr<Component> Register(GameObject& gameObject, const std::string& id) const = 0;

    const std::vector<std::string>& GetRegisteredComponentNames() const { return registeredComponentNames_; }

protected:    
    // コンストラクタで設定する
    const std::vector<std::string> registeredComponentNames_;

};

class DefaultComponentRegisterer :
    public ComponentRegisterer {
public:
    DefaultComponentRegisterer();
    std::shared_ptr<Component> Register(GameObject& gameObject, const std::string& id) const override;

};