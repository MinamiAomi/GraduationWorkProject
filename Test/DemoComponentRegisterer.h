#pragma once

#include "GameObject/ComponentRegisterer.h"

class DemoComponentRegisterer :
    public ComponentRegisterer {
public:
    /// <summary>
    /// 
    /// </summary>
    DemoComponentRegisterer();
    /// <summary>
    /// コンポーネントを追加
    /// </summary>
    /// <param name="gameObject"></param>
    /// <param name="id"></param>
    /// <returns></returns>
    std::shared_ptr<Component> Register(GameObject& gameObject, const std::string& id) const override;

};