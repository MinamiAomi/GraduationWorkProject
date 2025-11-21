#include "SceneObjectManager.h"
#include "Framework/AssetManager.h"

#include "SceneObjectConverter.h"

#ifdef _DEBUG
#include "Graphics/RenderManager.h"
#endif // _DEBUG

void SceneObjectSystem::SceneObjectManager::Initialize()
{
	sceneObjects_.clear();
	sceneObjectData_.clear();
}

void SceneObjectSystem::SceneObjectManager::CreateObjects(const std::vector<SceneObjectSystem::SceneObjectData>& objectData)
{

	const auto& assetManager = AssetManager::GetInstance();

	stageObjects_.SetModel(assetManager->modelMap.Get("Stage")->Get());
	stageObjects_.SetWorldMatrix(Matrix4x4::identity);
	for (const auto& obj : objectData) {

		auto sceneObject = std::make_unique<SceneObject>();

		sceneObject->model_.SetModel(assetManager->modelMap.Get(obj.modelName)->Get());

		//blender->左手座標系
		sceneObject->transform = obj.transform;
		sceneObject->transform.translate = SceneObjectSystem::SceneObjectConverter::ConvertTranslateToLeftHand(sceneObject->transform.translate);
		sceneObject->transform.rotate = SceneObjectSystem::SceneObjectConverter::ConvertRotateToLeftHand(sceneObject->transform.rotate);
		sceneObject->transform.UpdateMatrix();


		sceneObject->obbCollision = obj.obbCollision;
		if (sceneObject->obbCollision) {
			auto& obb = *sceneObject->obbCollision;

			obb.center = SceneObjectSystem::SceneObjectConverter::ConvertTranslateToLeftHand(obb.center);
			obb.rotation = SceneObjectSystem::SceneObjectConverter::ConvertRotateToLeftHand(obb.rotation);
		}


		sceneObject->isEmissive = obj.isEmissive;

		sceneObjects_.push_back(std::move(sceneObject));

		sceneObjectData_.emplace_back(std::make_unique<SceneObjectData>(obj));
	}
}

void SceneObjectSystem::SceneObjectManager::Update()
{

	for (const auto& obj : sceneObjects_) {
		obj->transform.UpdateMatrix();
		obj->model_.SetWorldMatrix(obj->transform.worldMatrix);
	}
#ifdef _DEBUG
	auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();

	const Vector4 obbColor = { 0.0f, 1.0f, 0.0f, 1.0f };

	for (const auto& obj : sceneObjects_) {
		const auto& obb = obj->obbCollision;
		lineDrawer.ObbDraw(obb->center, obb->size, obb->rotation, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	}
#endif // _DEBUG
}
