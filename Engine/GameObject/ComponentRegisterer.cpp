#include "ComponentRegisterer.h"

DefaultComponentRegisterer::DefaultComponentRegisterer() :
    ComponentRegisterer({}) {
}

std::shared_ptr<Component> DefaultComponentRegisterer::Register(GameObject& gameObject, const std::string& id) const {
    gameObject, id;
    return std::shared_ptr<Component>();
}
