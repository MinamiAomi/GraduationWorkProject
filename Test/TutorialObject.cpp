#include "TutorialObject.h"

#include "Framework/AssetManager.h"



#ifdef _DEBUG
#include "Graphics/ImGuiManager.h"
#endif // _DEBUG

TutorialObject::TutorialObject()
{
	collider_ = std::make_shared<SphereCollider>(
		CollisionCategory::TUTORIAL,
		CollisionCategory::FLASHLIGHT,
		Vector3::zero,
		0.0f
	);
}

void TutorialObject::Initialize(const std::string& name, float frame)
{

	collider_->center = transform_.translate;
	collider_->radius = 0.0f;

#ifdef _DEBUG
	name_ = name;
#endif // _DEBUG

	auto assetManager = AssetManager::GetInstance();

	auto texture = assetManager->textureMap.Get(name)->Get();

	sprite_.SetTexture(texture);
	sprite_.SetColor(Color::white);
	sprite_.SetPosition({ 1280.0f * 0.5f, 720.0f * 0.5f });
	sprite_.SetScale(texture->GetSize());
	sprite_.SetAnchor({ 0.5f, 0.5f });
	sprite_.SetUVRect({ {0.0f,0.0f} ,{1.0f,1.0f} }, Sprite::UVMode::UV);
	sprite_.SetIsActive(false);
	sprite_.SetDrawOrder(5);

	isOnce_ = false;

	drawFrame_ = frame;

	maxTime_ = 120.0f;
	currentTime_ = maxTime_;
}

void TutorialObject::Update()
{
	//一度出現してスプライトが出ていなけらば用済み
	if (isOnce_ && !sprite_.GetIsActive()) {
		return;
	}

	if (!isOnce_ && railAnimationPlayer_->GetCurrentFrame() >= drawFrame_) {

		isOnce_ = true;

		sprite_.SetIsActive(true);

		transform_.SetParent(&railAnimationPlayer_->GetTransform());
		colliderOffset_ = { 0.0,-1.0f,8.0f };
		transform_.translate = colliderOffset_;
		transform_.UpdateMatrix();

		collider_->center = transform_.worldMatrix.GetTranslate();
		collider_->radius = 0.7f;
	}
	OnCollision();

#ifdef _DEBUG
	DrawImGui();
#endif // _DEBUG

}

void TutorialObject::OnCollision()
{
	if (!collider_->GetCollidedWith().empty()) {
		if (sprite_.GetIsActive()) {
			currentTime_--;
			if (currentTime_ <= 0.0f) {
				sprite_.SetIsActive(false);
			}
			return;
		}
	}
	currentTime_ = maxTime_;
}

#ifdef _DEBUG
void TutorialObject::DrawImGui()
{
	ImGui::Begin(name_.c_str());
	ImGui::DragFloat3("colliderOffset", &colliderOffset_.x);
	ImGui::DragFloat("radius", &collider_->radius);
	ImGui::End();

	transform_.translate = colliderOffset_;
	transform_.UpdateMatrix();
	collider_->center = transform_.worldMatrix.GetTranslate();

	sprite_.DrawImGui(name_ + "sprite");
}
#endif // _DEBUG
