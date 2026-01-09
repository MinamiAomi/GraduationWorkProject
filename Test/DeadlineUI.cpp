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
	monster_.SetWorldMatrix(deadline_->GetAnimationPlayer()->EvaluateRailTransform(deadline_->GetCurrenFrame()).worldMatrix);
	monster_.SetIsActive(false);
}

void DeadlineUI::Update()
{
	if (!monster_.IsActive() &&
		deadline_->GetCurrenFrame() >= deadline_->GetStartFrame()) {
		monster_.SetIsActive(true);
	}

	monsterRotateTimer_ += 0.1f;

	Quaternion rollingRotation = Quaternion::MakeForXAxis(monsterRotateTimer_);

	Matrix4x4 railMatrix = deadline_->GetAnimationPlayer()->EvaluateRailTransform(deadline_->GetCurrenFrame()).worldMatrix;
	Vector3 railPos = railMatrix.GetTranslate();
	Quaternion railRot = railMatrix.GetRotate();

	Quaternion finalRotation = railRot * rollingRotation;

	Matrix4x4 finalMatrix = Matrix4x4::MakeAffineTransform(monsterTransform_.scale, finalRotation, railPos);

	monster_.SetWorldMatrix(finalMatrix);
}