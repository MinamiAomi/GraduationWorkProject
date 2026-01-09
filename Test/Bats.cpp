#include "Bats.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"

Bats::Bats(const std::vector<std::vector<bool>>& data)
{
	auto assetManager = AssetManager::GetInstance();
	model_ = assetManager->modelMap.Get("Box")->Get();
	radius_ = 5.0f;                       // Sphere用半径
	material_ = std::make_shared<Material>();

	for (size_t r = 0; r < data.size(); ++r) {
		for (size_t c = 0; c < data[r].size(); ++c) {
			if (data[r][c] == true) {
				Emit(r, c);
			}
		}
	}
}

void Bats::Update()
{
	
	transform_.UpdateMatrix();

	for (auto it = bats_.begin(); it != bats_.end(); ) {

		Bat* p = it->get(); 

		if (p->transform_.scale.x <= 0.0f) {
			it = bats_.erase(it);
		}
		else {
			// 行列更新
			p->transform_.UpdateMatrix();
			p->modelInstance_.SetWorldMatrix(p->transform_.worldMatrix);

			++it;
		}
	}

}

void Bats::DebugDraw()
{
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();
	lineDrawer.DrawSphere(transform_.worldMatrix.GetTranslate(), radius_, Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f});
}

void Bats::Emit(size_t r, size_t c)
{
	auto newBat = std::make_unique<Bat>();

	newBat->modelInstance_.SetModel(model_);
	newBat->modelInstance_.SetMaterial(material_);

	// 配列の要素数を保存
	newBat->rowIndex = r;
	newBat->colIndex = c;

	// 出現位置の計算（例としてランダムな球体内座標）
	Vector3 emitterWorldPos = transform_.worldMatrix.GetTranslate();
	Vector3 spawnPos = Vector3::zero;
	while (true) {
		Vector3 offset = {
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f),
			rnd_.NextFloatRange(-1.0f, 1.0f)
		};
		if (offset.LengthSquare() <= 1.0f) {
			spawnPos = emitterWorldPos + (offset * radius_);
			break;
		}
	}
	newBat->transform_.translate = spawnPos;
	bats_.push_back(std::move(newBat));
}
