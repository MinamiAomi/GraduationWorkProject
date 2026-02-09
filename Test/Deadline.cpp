#include "Deadline.h"

#include "RailAnimationPlayer.h"

#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG


#include "LevelManager.h"

Deadline::Deadline()
{
	deadlineUI_.SetDeadline(this);
}

void Deadline::Initialize()
{
	JSON_OPEN("Resources/Data/Deadline/deadline.json");
	JSON_LOAD(minSpeed_);
	JSON_LOAD(maxSpeed_);
	JSON_LOAD(approachRate_);
	JSON_LOAD(startFrameLevel1_);
	JSON_LOAD(startFrameLevel2_);
	JSON_LOAD(startOffset_);
	JSON_ROOT();
	JSON_CLOSE();
	currentFrame_ = 0.0f;

	isGameOver_ = false;

	deadlineUI_.Initialize();
}

void Deadline::Update(float deltaTime)
{
	float playerCurrentFrame = railAnimationPlayer_->GetCurrentFrame();

	float startFrame = 0.0f;

	switch (LevelManager::GetInstance()->GetLevel())
	{
	case LevelManager::Level::LEVEL1:
		startFrame = startFrameLevel1_;
		break;
	case LevelManager::Level::LEVEL2:
		startFrame = startFrameLevel2_;
		break;
	default:
		break;
	}

	if (!railAnimationPlayer_->IsFinished() &&
		railAnimationPlayer_->IsPlaying() &&
		startFrame <= playerCurrentFrame) {
		if (currentFrame_ < startFrame) {
			currentFrame_ = startFrame - startOffset_;
		}

		float distance = playerCurrentFrame - currentFrame_;

		float targetSpeed = minSpeed_ + (distance * approachRate_);

		currentActualSpeed_ = std::clamp(targetSpeed, minSpeed_, maxSpeed_);

		currentFrame_ += currentActualSpeed_ * deltaTime;

		if (currentFrame_ >= railAnimationPlayer_->GetCurrentFrame()) {
			isGameOver_ = true;
		}
	}

	deadlineUI_.Update();
#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

#ifdef _DEBUG
void Deadline::DrawImGui()
{
	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("ゲームオーバー制御（Deadline）")) {
		// 保存ボタン
		if (ImGui::Button("Save", ImVec2(-1.0f, 0.0f))) {
			JSON_OPEN("Resources/Data/Deadline/deadline.json");
			JSON_SAVE(minSpeed_);
			JSON_SAVE(maxSpeed_);
			JSON_SAVE(approachRate_);
			JSON_SAVE(startFrameLevel1_);
			JSON_SAVE(startFrameLevel2_);
			JSON_SAVE(startOffset_);
			JSON_ROOT();
			JSON_CLOSE();
		}

		ImGui::Separator();

		auto railAnimationData = railAnimationPlayer_->GetRailAnimationDate();
		ImGui::DragFloat("スタート位置オフセットLevel1", &startFrameLevel1_, 0.1f, float(railAnimationData->railMetaData_.startFrame), float(railAnimationData->railMetaData_.endFrame));
		ImGui::DragFloat("スタート位置オフセットLevel2", &startFrameLevel2_, 0.1f, float(railAnimationData->railMetaData_.startFrame), float(railAnimationData->railMetaData_.endFrame));
		ImGui::DragFloat("スタート位置オフセットからどのくらい後ろにするか", &startOffset_, 0.1f);

		ImGui::Separator();
		ImGui::Text("--- 追跡パラメータ調整 ---");

		// プランナー用パラメータ
		ImGui::DragFloat("最低速度 (Min Speed)", &minSpeed_, 0.01f, 0.0f, maxSpeed_, "%.2f");
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("プレイヤーが近くにいても、この速度は維持します");

		ImGui::DragFloat("最高速度 (Max Speed)", &maxSpeed_, 0.1f, minSpeed_, 100.0f, "%.2f");
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("プレイヤーがどれだけ遠くても、この速度以上は出しません");

		ImGui::DragFloat("追跡強度 (Rate)", &approachRate_, 0.01f, 0.0f, 10.0f, "%.2f");
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("値を大きくすると、距離が開いた時に猛スピードで追いかけます");

		ImGui::Separator();

		// デバッグ情報（リードオンリー）
		float distance = railAnimationPlayer_->GetCurrentFrame() - currentFrame_;
		ImGui::Text("現在の距離: %.2f フレーム分", distance);
		ImGui::Text("現在の速度: %.2f / sec", currentActualSpeed_);

		// 可視化（ゲージ）: 距離が近いほど赤くなるなどの演出も可能
		ImGui::ProgressBar(1.0f - (distance / 100.0f), ImVec2(-1.0f, 0.0f));

		ImGui::TreePop();
	}

	ImGui::End();
}
#endif // _DEBUG