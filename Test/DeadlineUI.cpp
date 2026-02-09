#include "DeadlineUI.h"

#include "Deadline.h"

#include "Framework/AssetManager.h"

#include "Engine/File/JsonConverter.h"

#include "RailAnimationPlayer.h"

#include "LevelManager.h"
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
	JSON_OPEN("Resources/Data/DeadlineUI/deadlineUI.json");
	JSON_LOAD(startLevel1Warning_);
	JSON_LOAD(startLevel2Warning_);
	JSON_CLOSE();
	monster_.SetWorldMatrix(deadline_->GetAnimationPlayer()->EvaluateRailTransform(deadline_->GetCurrenFrame()).worldMatrix);
	monster_.SetIsActive(false);
	warning_.SetIsActive(false);
	count_ = 0;
	isOnce_ = false;
}

void DeadlineUI::Update()
{
	float startWarning = 0.0f;

	switch (LevelManager::GetInstance()->GetLevel())
	{
	case LevelManager::Level::LEVEL1:
		startWarning = startLevel1Warning_;
		break;
	case LevelManager::Level::LEVEL2:
		startWarning = startLevel2Warning_;
		break;
	default:
		break;
	}

	if (!isOnce_ &&
		deadline_->GetAnimationPlayer()->GetCurrentFrame() > startWarning) {

		warning_.SetIsActive(true);
		count_ = 150;
		isOnce_ = true;
	}

	if (warning_.GetIsActive() &&
		isOnce_ &&
		count_ <= 0) {
		warning_.SetIsActive(false);
	}
	else {
		count_--;
	}

	if (!monster_.IsActive() &&
		deadline_->GetCurrenFrame() > 0.0f) {
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

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}
#ifdef _DEBUG
void DeadlineUI::DrawImGui()
{
	ImGui::Begin("GameScene");

	if (ImGui::TreeNode("ゲームオーバー制御UI（DeadlineUI）")) {
		// 保存ボタン
		if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
			JSON_OPEN("Resources/Data/DeadlineUI/deadlineUI.json");
			JSON_SAVE(startLevel1Warning_);
			JSON_SAVE(startLevel2Warning_);
			JSON_CLOSE();
		}

		ImGui::Separator();
		ImGui::DragFloat("Level1の危険UI表示タイミング", &startLevel1Warning_);
		ImGui::DragFloat("Level2の危険UI表示タイミング", &startLevel2Warning_);

		ImGui::TreePop();
	}

	ImGui::End();
}
#endif // _DEBUG