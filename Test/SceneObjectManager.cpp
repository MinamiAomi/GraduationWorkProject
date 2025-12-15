#include "SceneObjectManager.h"

#include "RailConverter.h"
#include "SceneObjectConverter.h"

#include "Engine/File/JsonConverter.h"

#ifdef _DEBUG
#include "Graphics/RenderManager.h"
#endif // _DEBUG

void SceneObjectSystem::SceneObjectManager::Initialize()
{
	pointLightObjects_.clear();
	emitterObjects_.clear();
	enemyObjects_.clear();
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

		auto& col = convertedObj->capsuleCollisionData.value();
		col.center = SceneObjectConverter::ConvertTranslateToLeftHand(rawObj.capsuleCollisionData->center);
		col.quaternion = SceneObjectConverter::ConvertRotateToLeftHand(rawObj.capsuleCollisionData->quaternion);

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
	emitterObjects_.clear();
	enemyObjects_.clear();

	BuildRuntimeObjects();

}

void SceneObjectSystem::SceneObjectManager::Update()
{
	for (const auto& obj : pointLightObjects_) {
		obj->transform.UpdateMatrix();
		obj->model.SetWorldMatrix(obj->transform.worldMatrix);
		obj->lightObject.Update();
		//何かに当ったら
		if (obj->collider &&
			!obj->collider->GetCollidedWith().empty()) {
			obj->collider = nullptr;
			obj->lightObject.SetDamage(sceneObjectConfig_.pointLightParams.damageReceived);
		}
	}

	for (const auto& obj : emitterObjects_) {
		obj->transform.UpdateMatrix();
		obj->model.SetWorldMatrix(obj->transform.worldMatrix);


	}


	for (const auto& obj : enemyObjects_) {
		obj->transform.UpdateMatrix();
		obj->model.SetWorldMatrix(obj->transform.worldMatrix);
	}

#ifdef _DEBUG
	sceneObjectConfig_.DrawImGui();
#endif // _DEBUG

}

void SceneObjectSystem::SceneObjectManager::BuildRuntimeObjects()
{
	sceneObjectConfig_.Initialize();


	for (const auto& objDataPtr : sceneObjectData_) {
		const auto& data = *objDataPtr;

		switch (data.type)
		{
		case SceneObjectSystem::ObjectType::PointLight:
		{
			auto pointLightObject = std::make_unique<PointLightObject>();

			const auto& assetManager = AssetManager::GetInstance();

			if (auto modelHandle = assetManager->modelMap.Get(data.modelName)) {
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


			InitializeCommonObject(pointLightObject, data);

			//ライトの設定
			if (data.pointLightData) {
				pointLightObject->lightObject.Initialize(&pointLightObject->transform);

				//Hp設定
				float scale = std::max(std::max(pointLightObject->transform.scale.x, pointLightObject->transform.scale.y), pointLightObject->transform.scale.z);
				float maxHp = sceneObjectConfig_.pointLightParams.baseHp * (1.0f * (scale - 1.0f) * sceneObjectConfig_.pointLightParams.sizeCorrectionFactor);
				pointLightObject->lightObject.SetMaxHp(maxHp);
				pointLightObject->lightObject.SetHp(maxHp);

				PointLight pointLight;
				pointLight.intensity = data.pointLightData->intensity;
				pointLight.range = data.pointLightData->range;
				pointLight.decay = 1.0f;
				pointLight.color = data.pointLightData->color;

				pointLightObject->lightObject.SetLightSetting(pointLight);
				pointLightObject->lightObject.SetOffset(data.pointLightData->offset);
			}

			pointLightObjects_.push_back(std::move(pointLightObject));
		}
		break;

		case SceneObjectSystem::ObjectType::Emitter:
		{
			auto emitterObject = std::make_unique<EmitterObject>();

			const auto& assetManager = AssetManager::GetInstance();

			if (auto modelHandle = assetManager->modelMap.Get(data.modelName)) {
				emitterObject->model.SetModel(modelHandle->Get());
			}


			InitializeCommonObject(emitterObject, data);

			emitterObjects_.push_back(std::move(emitterObject));
		}
		break;

		case SceneObjectSystem::ObjectType::Enemy:
		{
			auto enemyObject = std::make_unique<EnemyObject>();

			const auto& assetManager = AssetManager::GetInstance();

			if (auto modelHandle = assetManager->modelMap.Get(data.modelName)) {
				enemyObject->model.SetModel(modelHandle->Get());
			}


			InitializeCommonObject(enemyObject, data);

			enemyObjects_.push_back(std::move(enemyObject));
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
			ImGui::SliderFloat("確認用スケール", &debugScale, 1.0f, 5.0f);

			// ※実際のゲーム内の計算式に合わせてください。ここでは仮の式です
			// 例: HP = 基準HP * (1 + (スケール - 1) * 補正係数)
			float finalHp = params->baseHp * (1.0f + (debugScale - 1.0f) * params->sizeCorrectionFactor);

			ImGui::Text("結果HP: %.1f (＝ 耐久回数: %.1f回)", finalHp, finalHp / (params->damageReceived + 0.0001f));

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