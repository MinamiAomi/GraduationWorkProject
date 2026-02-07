#include "SceneObjectManager.h"

#include "RailConverter.h"
#include "SceneObjectConverter.h"

#include "Engine/File/JsonConverter.h"

#include "Trolley.h"

#ifdef _DEBUG
#include "Graphics/RenderManager.h"
#endif // _DEBUG

#include "Bats.h"

void SceneObjectSystem::SceneObjectManager::Initialize()
{
	pointLightObjects_.clear();
	enemySpawnObjects_.clear();
	gimmickMoverObjects_.clear();
	gimmickTriggerObjects_.clear();
	obstacleObjects_.clear();

	sceneObjectData_.clear();
}

void SceneObjectSystem::SceneObjectManager::CreateObjects(const std::vector<SceneObjectSystem::SceneObjectData>& objectData)
{
	sceneObjectData_.clear();

	const auto& assetManager = AssetManager::GetInstance();
	stageObjects_.SetModel(assetManager->modelMap.Get("Stage")->Get());
	stageObjects_.SetWorldMatrix(Matrix4x4::identity);

	for (const auto& rawObj : objectData) {
		auto convertedObj = std::make_unique<SceneObjectData>(rawObj);

		convertedObj->transform.scale = SceneObjectConverter::ConvertSizeToLeftHand(rawObj.transform.scale);
		convertedObj->transform.rotate = SceneObjectConverter::ConvertRotateToLeftHand(rawObj.transform.rotate);
		convertedObj->transform.translate = SceneObjectConverter::ConvertTranslateToLeftHand(rawObj.transform.translate);

		if (convertedObj->capsuleCollisionData) {
			auto& col = convertedObj->capsuleCollisionData.value();
			col.center = SceneObjectConverter::ConvertTranslateToLeftHand(rawObj.capsuleCollisionData->center);
			col.quaternion = SceneObjectConverter::ConvertRotateToLeftHand(rawObj.capsuleCollisionData->quaternion);
		}

		if (convertedObj->sphereCollisionData) {
			auto& col = convertedObj->sphereCollisionData.value();
			col.center = SceneObjectConverter::ConvertTranslateToLeftHand(rawObj.sphereCollisionData->center);
		}

		if (convertedObj->pointLightData) {
			convertedObj->pointLightData->offset = SceneObjectConverter::ConvertTranslateToLeftHand(convertedObj->pointLightData->offset);
		}

		sceneObjectData_.push_back(std::move(convertedObj));
	}

	BuildRuntimeObjects();
}

void SceneObjectSystem::SceneObjectManager::ResetObjects()
{
	pointLightObjects_.clear();
	enemySpawnObjects_.clear();
	gimmickMoverObjects_.clear();
	gimmickTriggerObjects_.clear();
	gimmickPointLightObjects_.clear();
	obstacleObjects_.clear();

	BuildRuntimeObjects();

}

void SceneObjectSystem::SceneObjectManager::Update()
{
	for (const auto& obj : pointLightObjects_) {
		obj->Update();
		//何かに当ったら
		if (obj->collider &&
			!obj->collider->GetCollidedWith().empty()) {
			obj->lightObject.SetDamage(sceneObjectConfig_.pointLightParams.damageReceived);
			if (!obj->lightObject.GetIsAlive()) {
				obj->collider = nullptr;
			}

		}
	}

	for (const auto& obj : enemySpawnObjects_) {
		if (obj->collider &&
			obj->collider->GetCollidedWith().empty()) {
			//スポーン
			batsManager_->Emit(obj->formation);
			obj->collider = nullptr;
		}
	}


	for (const auto& obj : gimmickTriggerObjects_) {
		if (obj->collider &&
			!obj->collider->GetCollidedWith().empty()) {

			//一回しか発動しないかどうか
			if (obj->hasTriggered && obj->isOnce) { return; }

			for (const auto& mover : gimmickMoverObjects_) {
				if (mover->key == obj->key) {
					mover->isActive = true;
				}
			}

			for (const auto& light : gimmickPointLightObjects_) {
				if (light->mover.key == obj->key) {
					light->mover.isActive = true;
				}
			}

			obj->hasTriggered = obj->isOnce;
		}
	}

	for (const auto& obj : gimmickMoverObjects_) {
		if (obj->isActive) {
			obj->Update();
		}
	}

	for (const auto& obj : gimmickPointLightObjects_) {
		auto& pointlight = obj->pointlight;
		auto& mover = obj->mover;

		mover.Update(obj->transform);
		pointlight.Update();
		obj->model.SetIsActive(mover.isActive);
		if (mover.isActive) {
			obj->model.SetWorldMatrix(obj->transform.worldMatrix);
		}
		//何かに当ったら
		if (obj->collider &&
			!obj->collider->GetCollidedWith().empty()) {
			for (auto& collider : obj->collider->GetCollidedWith()) {
				if ((collider->categoryBits == CollisionCategory::FLASHLIGHT)) {
					pointlight.lightObject.SetDamage(sceneObjectConfig_.pointLightParams.damageReceived);
					if (!pointlight.lightObject.GetIsAlive()) {
						obj->collider = nullptr;
					}
				}
			}
		}
	}

	for (const auto& obj : obstacleObjects_) {


		obj->model.SetWorldMatrix(obj->transform.worldMatrix);
		obj->model.SetIsActive(true);

		if (obj->collider &&
			!obj->collider->GetCollidedWith().empty()) {

			for (auto& collider : obj->collider->GetCollidedWith()) {
				if (collider->categoryBits == CollisionCategory::FLASHLIGHT) {
					obj->SetDamage();
					float t = obj->hp / obj->maxHp;
					t = 1.0f - float(std::sqrt(1.0f - std::pow(t, 2)));
					obj->transform.scale = Vector3(t, t, t);
					obj->transform.UpdateMatrix();
					if (!obj->isAlive) {
						obj->collider = nullptr;
					}
				}
				else if ((collider->categoryBits == CollisionCategory::PLAYER)) {
					if (obj->isAlive) {
						Trolley::GetInstance()->SetState(Trolley::State::Burst);
						obj->model.SetIsActive(false);
						obj->model.SetWorldMatrix(obj->transform.worldMatrix);
						obj->collider = nullptr;
					}
				}
			}
		}
	}

#ifdef _DEBUG
	sceneObjectConfig_.DrawImGui();
#endif // _DEBUG

}

void SceneObjectSystem::SceneObjectManager::BuildRuntimeObjects()
{
	sceneObjectConfig_.Initialize();

	uint32_t myCategory, targetMask;

	for (const auto& objDataPtr : sceneObjectData_) {
		const auto& data = *objDataPtr;

		switch (data.type)
		{
		case SceneObjectSystem::ObjectType::PointLight:
		{
			auto pointLightObject = std::make_unique<PointLightObject>();

			const auto& assetManager = AssetManager::GetInstance();

			if (auto modelHandle = assetManager->modelMap.Get(data.modelName.value())) {
				pointLightObject->model.SetModel(modelHandle->Get());
				// エラーマテリアル
				pointLightObject->material = std::make_shared<Material>();
				pointLightObject->material->albedo = { 1.0f, 0.2f, 0.6f };
				pointLightObject->material->emissive = { 1.0f, 1.0f, 1.0f };

				// 発光マテリアルを設定
				if (!modelHandle->Get()->GetMaterials().empty()) {
					auto& originalMaterials = modelHandle->Get()->GetMaterials().at(0);
					(*pointLightObject->material) = originalMaterials;
					pointLightObject->material->emissive = { 1.0f, 1.0f, 1.0f };
				}
				pointLightObject->model.SetMaterial(pointLightObject->material);
			}
			myCategory = uint32_t(CollisionCategory::LIGHT);
			targetMask = uint32_t(CollisionCategory::FLASHLIGHT);

			InitializeCommonObject(pointLightObject, data, myCategory, targetMask);

			//ライトの設定
			if (data.pointLightData) {
				pointLightObject->lightObject.Initialize(&pointLightObject->transform);
				pointLightObject->lightObject.SetModel(pointLightObject->model.GetModel());

				float scale = std::max({
					pointLightObject->transform.scale.x,
					pointLightObject->transform.scale.y,
					pointLightObject->transform.scale.z
					});

				//基準の1と比べる
				float scaleDiff = scale - 1.0f;

				float multiplier = 1.0f + (scaleDiff * sceneObjectConfig_.pointLightParams.sizeCorrectionFactor);

				multiplier = std::max(multiplier, 0.1f);

				float maxHp = sceneObjectConfig_.pointLightParams.baseHp * multiplier;

				pointLightObject->lightObject.SetMaxHp(maxHp);
				pointLightObject->lightObject.SetHp(maxHp);

				PointLight pointLight;
				pointLight.intensity = data.pointLightData->intensity;
				pointLight.range = data.pointLightData->range;
				pointLight.decay = 1.0f;
				pointLight.color = data.pointLightData->color;

				pointLightObject->lightObject.SetLightSetting(pointLight);
				pointLightObject->lightObject.SetOffset(data.pointLightData->offset);

				pointLightObject->powerEmitter_.Initialize(EmitShape::kSphere, &pointLightObject->lightObject);
			}

			pointLightObjects_.push_back(std::move(pointLightObject));
		}
		break;
		case SceneObjectSystem::ObjectType::EnemySpawn:
		{
			auto enemySpawn = std::make_unique<EnemySpawnData>();

			enemySpawn->formation = data.enemySpawnData->formation;
			enemySpawn->hasTriggered = false;
			enemySpawn->isOnce = data.enemySpawnData->isOnce;

			myCategory = uint32_t(CollisionCategory::ENEMY);
			targetMask = uint32_t(CollisionCategory::FLASHLIGHT);

			InitializeCommonObject(enemySpawn, data, myCategory, targetMask);

			enemySpawnObjects_.push_back(std::move(enemySpawn));
		}
		break;

		case SceneObjectSystem::ObjectType::Gimmick:
		{
			auto mover = std::make_unique<GimmickMoverObject>();
			auto trigger = std::make_unique<GimmickTriggerObject>();
			auto pointlight = std::make_unique<GimmickPointLightObject>();

			if (data.gimmickMovers.has_value()) {
				const auto& assetManager = AssetManager::GetInstance();
				if (auto modelHandle = assetManager->modelMap.Get(data.modelName.value())) {
					mover->model.SetModel(modelHandle->Get());
				}

				mover->key = data.gimmickMovers->key;
				mover->duration = data.gimmickMovers->duration;
				mover->moverAnimation = data.gimmickMovers->moverAnimation;
				mover->evalTimeKeys = data.gimmickMovers->evalTimeKeys;
				mover->isCyclic = data.gimmickMovers->isCyclic;
				mover->isActive = false;

				myCategory = uint32_t(CollisionCategory::GIMMICKMOVER);
				targetMask = uint32_t(CollisionCategory::NONE);

				InitializeCommonObject(mover, data, myCategory, targetMask);
				gimmickMoverObjects_.push_back(std::move(mover));
			}
			else if (data.gimmickTriggers.has_value()) {

				trigger->key = data.gimmickTriggers->key;
				trigger->hasTriggered = false;
				trigger->isOnce = data.gimmickTriggers->isOnce;

				myCategory = uint32_t(CollisionCategory::GIMMICKTRIGGER);
				targetMask = uint32_t(CollisionCategory::PLAYER);

				InitializeCommonObject(trigger, data, myCategory, targetMask);
				gimmickTriggerObjects_.push_back(std::move(trigger));
			}
			else if (data.gimmickPointlights.has_value()) {
				const auto& assetManager = AssetManager::GetInstance();
				if (auto modelHandle = assetManager->modelMap.Get(data.modelName.value())) {
					pointlight->model.SetModel(modelHandle->Get());
				}
				const auto& moverData = data.gimmickPointlights->gimmickMoverData;

				pointlight->mover.key = moverData.key;
				pointlight->mover.duration = moverData.duration;
				pointlight->mover.moverAnimation = moverData.moverAnimation;
				pointlight->mover.evalTimeKeys = moverData.evalTimeKeys;
				pointlight->mover.isCyclic = moverData.isCyclic;
				pointlight->mover.isActive = false;

				//ライトの設定
				if (data.gimmickPointlights) {
					pointlight->pointlight.lightObject.Initialize(&pointlight->transform);
					pointlight->pointlight.lightObject.SetModel(pointlight->model.GetModel());

					float scale = std::max({
						pointlight->transform.scale.x,
						pointlight->transform.scale.y,
						pointlight->transform.scale.z
						});

					//基準の1と比べる
					float scaleDiff = scale - 1.0f;

					float multiplier = 1.0f + (scaleDiff * sceneObjectConfig_.pointLightParams.sizeCorrectionFactor);

					multiplier = std::max(multiplier, 0.1f);

					float maxHp = sceneObjectConfig_.pointLightParams.baseHp * multiplier;

					pointlight->pointlight.lightObject.SetMaxHp(maxHp);
					pointlight->pointlight.lightObject.SetHp(maxHp);

					PointLight pointLight;
					pointLight.intensity = data.gimmickPointlights->pointlightData.intensity;
					pointLight.range = data.gimmickPointlights->pointlightData.range;
					pointLight.decay = 1.0f;
					pointLight.color = data.gimmickPointlights->pointlightData.color;

					pointlight->pointlight.lightObject.SetLightSetting(pointLight);
					pointlight->pointlight.lightObject.SetOffset(data.gimmickPointlights->pointlightData.offset);

					pointlight->pointlight.powerEmitter.Initialize(EmitShape::kSphere, &pointlight->pointlight.lightObject);
				}

				myCategory = uint32_t(CollisionCategory::GIMMICKPOINTLIGHT);
				targetMask = uint32_t(CollisionCategory::FLASHLIGHT);

				InitializeCommonObject(pointlight, data, myCategory, targetMask);
				gimmickPointLightObjects_.push_back(std::move(pointlight));
			}
		}
		break;

		case SceneObjectSystem::ObjectType::Obstacle:
		{

			auto obstacleObject = std::make_unique<ObstacleObject>();

			const auto& assetManager = AssetManager::GetInstance();

			if (auto modelHandle = assetManager->modelMap.Get(data.modelName.value())) {
				obstacleObject->model.SetModel(modelHandle->Get());
			}

			obstacleObject->hp = data.obstacles->hp;
			obstacleObject->maxHp = obstacleObject->hp;
			obstacleObject->isAlive = true;

			myCategory = uint32_t(CollisionCategory::OBSTACLE);
			targetMask = uint32_t(CollisionCategory::PLAYER | CollisionCategory::FLASHLIGHT);

			InitializeCommonObject(obstacleObject, data, myCategory, targetMask);
			obstacleObjects_.push_back(std::move(obstacleObject));
		}
		break;
		default:
			break;
		}
	}
}

#ifdef _DEBUG
// ヘルパー関数：？マークを出してホバー時に説明を表示
static void HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void SceneObjectSystem::SceneObjectManager::SceneObjectConfig::DrawImGui()
{
	ImGui::Begin("GameScene");

	if (ImGui::TreeNode("シーンオブジェクト設定 (SceneObjectConfig)")) {

		if (ImGui::Button("Save ", ImVec2(-1.0f, 0.0f))) {
			JSON_OPEN("Resources/Data/SceneObjectManager/sceneObjectManager.json");
			JSON_OBJECT("pointLight");
			JSON_SAVE(pointLightParams.baseHp);
			JSON_SAVE(pointLightParams.sizeCorrectionFactor);
			JSON_SAVE(pointLightParams.damageReceived);
			JSON_ROOT();
			JSON_CLOSE();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("ポイントライト設定")) {
			// パラメータへのポインタを取得してコードを見やすく
			auto* params = &pointLightParams;

			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "■ 基本ステータス");

			// 基準HP (0以下にならないようにminを設定)
			ImGui::DragFloat("基準HP", &params->baseHp, 1.0f, 1.0f, 10000.0f, "%.0f");
			ImGui::SameLine(); HelpMarker("オブジェクトのスケールが (1,1,1) の時のHPです。");

			// サイズ補正係数 (0.0～10.0くらいに制限)
			ImGui::DragFloat("サイズ補正係数", &params->sizeCorrectionFactor, 0.01f, 0.0f, 10.0f);
			ImGui::SameLine(); HelpMarker("オブジェクトの大きさに応じてHPをどれくらい増やすか。\n0.0: 大きさに関係なく基準HPのまま\n1.0: 大きさに比例してHPが増える");

			// 被ダメージ
			ImGui::DragFloat("被ダメージ量", &params->damageReceived, 0.1f, 0.0f, 100.0f);
			ImGui::SameLine(); HelpMarker("ライトを照らしたときに減るHP量");

			ImGui::Separator();

			// --- 計算プレビュー機能 ---
			// プランナーが「結局HPいくらになるの？」を確認するためのシミュレーター
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "■ HP計算プレビュー");
			static float debugScale = 2.0f; // テスト用のスケール
			ImGui::SliderFloat("確認用スケール", &debugScale, 0.0f, 5.0f);


			float scaleDiff = debugScale - 1.0f;

			float multiplier = 1.0f + (scaleDiff * params->sizeCorrectionFactor);

			multiplier = std::max(multiplier, 0.1f);

			float finalHp = params->baseHp * (1.0f + (debugScale - 1.0f) * params->sizeCorrectionFactor);

			ImGui::Text("結果HP: %.1f", finalHp);

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	ImGui::End();
}
#endif // _DEBUG

void SceneObjectSystem::SceneObjectManager::SceneObjectConfig::Initialize()
{
	JSON_OPEN("Resources/Data/SceneObjectManager/sceneObjectManager.json");
	JSON_OBJECT("pointLight");

	JSON_LOAD(pointLightParams.baseHp);
	JSON_LOAD(pointLightParams.sizeCorrectionFactor);
	JSON_LOAD(pointLightParams.damageReceived);

	JSON_ROOT();
	JSON_CLOSE();
}