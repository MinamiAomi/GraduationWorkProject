#include "DeadlineUI.h"

#include "Deadline.h"

#include "Framework/AssetManager.h"

#include "RailAnimationPlayer.h"
DeadlineUI::DeadlineUI()
{
	auto assetManager = AssetManager::GetInstance();
	monster_.SetModel(assetManager->modelMap.Get("monster")->Get());
}

void DeadlineUI::Initialize()
{
	monster_.SetWorldMatrix(deadline_->GetAnimationPlayer()->EvaluateTransform(deadline_->GetCurrenFrame()).worldMatrix);
}

void DeadlineUI::Update()
{
	monster_.SetWorldMatrix(deadline_->GetAnimationPlayer()->EvaluateTransform(deadline_->GetCurrenFrame()).worldMatrix);
}
