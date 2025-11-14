#include "Trolley.h"

#include "Framework/AssetManager.h"

Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());
}

void Trolley::Initialize()
{
	offset_ = { 0.0f,-1.3f,0.0f };
	transform_.translate += offset_;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);
}

void Trolley::Update()
{
	transform_.translate += offset_;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);
}

void Trolley::SetTransform(const Transform& transform)
{
	transform_ = transform;
}
