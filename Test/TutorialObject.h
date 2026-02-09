#pragma once

#include <memory>
#include <string>

#include "Math/Transform.h"
#include "Collider.h"

#include "Graphics/Model.h"
#include "Graphics/Sprite.h"


class TutorialObject {
public:
	TutorialObject();

	void Initialize(const Transform& transform,const std::string& name);

	void Update();

	std::shared_ptr<SphereCollider> GetCollider() { return collider_; }
private:
	void OnCollision();
	ModelInstance model_;
	Sprite sprite_;
	Transform transform_;
	std::shared_ptr<SphereCollider> collider_;

	
#ifdef _DEBUG
	std::string name_;
	void DrawImGui();
#endif // _DEBUG

};