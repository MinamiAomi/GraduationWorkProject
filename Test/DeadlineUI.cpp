#include "DeadlineUI.h"

#include "Deadline.h"

#include "Framework/AssetManager.h"

#include "RailAnimationPlayer.h"
DeadlineUI::DeadlineUI()
{
	auto assetManager = AssetManager::GetInstance();
	monster_.SetModel(assetManager->modelMap.Get("monster")->Get());

	auto texture = assetManager->textureMap.Get("Warning")->Get();

	warning_.SetTexture(texture);

	warning_.SetPosition({ 1280.0f * 0.5f,720.0f * 0.5f });
	warning_.SetAnchor({ 0.5f,0.5f });
	warning_.SetScale({ texture->GetSize() });
	warning_.SetUVRect({ {0.0f,0.0f },{1.0f,1.0f} }, Sprite::UVMode::UV);

}

void DeadlineUI::Initialize()
{
	monster_.SetWorldMatrix(deadline_->GetAnimationPlayer()->EvaluateRailTransform(deadline_->GetCurrenFrame()).worldMatrix);
	monster_.SetIsActive(false);
		warning_.SetIsActive(false);
	count_ = 0;
}

void DeadlineUI::Update()
{
	if (!monster_.IsActive() &&
		deadline_->GetCurrenFrame() > 0.0f) {
		monster_.SetIsActive(true);
		warning_.SetIsActive(true);

		count_ = 120;
	}

	if (count_ <= 0 && warning_.GetIsActive()) {
		warning_.SetIsActive(false);
	}
	else {
		count_--;
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