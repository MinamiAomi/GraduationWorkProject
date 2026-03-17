#include "CollisionSystem.h"
#include "Engine/Graphics/ImGuiManager.h"

bool CollisionSystem::isCollisionDebugDraw = false;

void CollisionSystem::Initialize()
{
	isActive_ = true;
}

void CollisionSystem::CheckCollisions()
{
	if (!isActive_) {
		return; 
	}

#ifdef _DEBUG
	ImGui::Begin("GameScene");
	if (ImGui::TreeNode("Collision")) {
		ImGui::Checkbox("DebugDraw", &isCollisionDebugDraw);
		ImGui::TreePop();
	}
	ImGui::End();
#endif
	//死んでいるコライダー削除（俺が追加）
	colliders.erase(
		std::remove_if(colliders.begin(), colliders.end(),
			[](const std::weak_ptr<Collider>& wp) { return wp.expired(); }),
		colliders.end());

	for (const auto& weak_ptr : colliders) {
		if (auto ptr = weak_ptr.lock()) { // 生存確認
			ptr->ClearCollisionInfo();
		}
	}

	for (auto iter1 = colliders.begin(); iter1 != colliders.end(); ++iter1) {
		for (auto iter2 = std::next(iter1); iter2 != colliders.end(); ++iter2) {
			std::shared_ptr<Collider> a_ptr = iter1->lock();
			std::shared_ptr<Collider> b_ptr = iter2->lock();
			// 2つのColliderで衝突判定を実行
			if (a_ptr && b_ptr) {
				if (AreColliding(*a_ptr, *b_ptr)) {
					//情報をリストに貯める
					a_ptr->OnCollision(*b_ptr);
					b_ptr->OnCollision(*a_ptr);
				}
			}
		}
	}

	colliders.erase(
		std::remove_if(colliders.begin(), colliders.end(),
			[](const std::weak_ptr<Collider>& wp) {
				return wp.expired(); // 参照先が失効しているか？
			}),
		colliders.end()
	);

	if (isCollisionDebugDraw) {
		for (const auto& weak_ptr : colliders) {
			if (auto collider = weak_ptr.lock()) {

				Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
				if (!collider->GetCollidedWith().empty()) {
					color = { 1.0f, 0.0f, 0.0f, 1.0f };
				}
#ifdef _DEBUG
				collider->DrawDebug(color);
#endif // _DEBUG
			}
		}
	}
}

bool CollisionSystem::AreColliding(Collider& a, Collider& b)
{
	bool canCollide = (a.maskBits & static_cast<uint32_t>(b.categoryBits)) != 0 &&
		(b.maskBits & static_cast<uint32_t>(a.categoryBits)) != 0;

	if (!canCollide) {
		return false;
	}

	bool result = a.CheckCollision(b);

	return result;
}

void CollisionSystem::SetIsActive(bool flag)
{
	isActive_ = flag;
	for (const auto& weak_ptr : colliders) {
		if (auto ptr = weak_ptr.lock()) {
			ptr->ClearCollisionInfo();
		}
	}
}
