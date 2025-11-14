#include "Trolley.h"

#include "Framework/AssetManager.h"

Trolley::Trolley()
{
	model_.SetModel(AssetManager::GetInstance()->modelMap.Get("trolley")->Get());
}

void Trolley::Initialize()
{
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);
}

void Trolley::Update()
{
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);
}

void Trolley::SetTransform(const Transform& transform)
{
	transform_ = transform;
	transform_.UpdateMatrix();
	model_.SetWorldMatrix(transform_.worldMatrix);
}
