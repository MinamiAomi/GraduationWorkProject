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

	for (const auto& obj : objectData) {

		auto sceneObject = std::make_unique<SceneObject>();

		sceneObject->model_.SetModel(assetManager->modelMap.Get(obj.modelName)->Get());

		//blender->左手座標系
		sceneObject->transform = obj.transform;
		sceneObject->transform.translate = SceneObjectSystem::SceneObjectConverter::ConvertTranslateToLeftHand(sceneObject->transform.translate);
		sceneObject->transform.rotate = SceneObjectSystem::SceneObjectConverter::ConvertRotateToLeftHand(sceneObject->transform.rotate);
		sceneObject->transform.UpdateMatrix();


		sceneObject->obbCollision = obj.obbCollision;
		sceneObject->obbCollision.center = SceneObjectSystem::SceneObjectConverter::ConvertTranslateToLeftHand(sceneObject->obbCollision.center);
		sceneObject->obbCollision.rotation = SceneObjectSystem::SceneObjectConverter::ConvertRotateToLeftHand(sceneObject->obbCollision.rotation);


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
		Vector3 halfSize = obb.size * 0.5f;

		Vector3 localCorners[8] = {
			{ -halfSize.x, -halfSize.y, -halfSize.z },
			{ +halfSize.x, -halfSize.y, -halfSize.z },
			{ -halfSize.x, +halfSize.y, -halfSize.z },
			{ +halfSize.x, +halfSize.y, -halfSize.z },
			{ -halfSize.x, -halfSize.y, +halfSize.z },
			{ +halfSize.x, -halfSize.y, +halfSize.z },
			{ -halfSize.x, +halfSize.y, +halfSize.z },
			{ +halfSize.x, +halfSize.y, +halfSize.z }
		};

		std::array<Vector3, 8> worldCorners;
		for (int i = 0; i < 8; ++i) {
			Vector3 rotatedCorner = obb.rotation * localCorners[i];

			worldCorners[i] = obb.center + rotatedCorner;
		}

		lineDrawer.AddLine(worldCorners[0], worldCorners[1], obbColor);
		lineDrawer.AddLine(worldCorners[1], worldCorners[3], obbColor);
		lineDrawer.AddLine(worldCorners[3], worldCorners[2], obbColor);
		lineDrawer.AddLine(worldCorners[2], worldCorners[0], obbColor);

		lineDrawer.AddLine(worldCorners[4], worldCorners[5], obbColor);
		lineDrawer.AddLine(worldCorners[5], worldCorners[7], obbColor);
		lineDrawer.AddLine(worldCorners[7], worldCorners[6], obbColor);
		lineDrawer.AddLine(worldCorners[6], worldCorners[4], obbColor);

		lineDrawer.AddLine(worldCorners[0], worldCorners[4], obbColor);
		lineDrawer.AddLine(worldCorners[1], worldCorners[5], obbColor);
		lineDrawer.AddLine(worldCorners[2], worldCorners[6], obbColor);
		lineDrawer.AddLine(worldCorners[3], worldCorners[7], obbColor);
	}
#endif // _DEBUG
}
