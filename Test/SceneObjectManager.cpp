#include "SceneObjectManager.h"
#include "Framework/AssetManager.h"

#include "RailConverter.h"
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
	sceneObjects_.clear();
	sceneObjectData_.clear();

	const auto& assetManager = AssetManager::GetInstance();

	stageObjects_.SetModel(assetManager->modelMap.Get("Stage")->Get());
	stageObjects_.SetWorldMatrix(Matrix4x4::identity);
	for (const auto& obj : objectData) {

		auto sceneObject = std::make_unique<SceneObject>();

		sceneObject->model_.SetModel(assetManager->modelMap.Get(obj.modelName)->Get());

		//blender->左手座標系
		sceneObject->transform = obj.transform;
		sceneObject->transform.scale = SceneObjectConverter::ConvertSizeToLeftHand(sceneObject->transform.scale);
		sceneObject->transform.rotate = SceneObjectConverter::ConvertRotateToLeftHand(sceneObject->transform.rotate);
		sceneObject->transform.translate = SceneObjectConverter::ConvertTranslateToLeftHand(sceneObject->transform.translate);
		sceneObject->transform.UpdateMatrix();


		if (obj.capsuleCollisionData) {
			sceneObject->collider = std::make_shared<CapsuleCollider>(
				CollisionCategory::LIGHT,
				CollisionCategory::FLASHLIGHT,
				Vector3::zero,
				0.0f,
				0.0f,
				Quaternion::identity);
			sceneObject->collider->get()->center = SceneObjectConverter::ConvertTranslateToLeftHand(obj.capsuleCollisionData->center);
			sceneObject->collider->get()->quaternion = SceneObjectConverter::ConvertRotateToLeftHand(obj.capsuleCollisionData->quaternion);
			sceneObject->collider->get()->radius = obj.capsuleCollisionData->radius;
			sceneObject->collider->get()->height = obj.capsuleCollisionData->height;
		}

		sceneObject->isEmissive = obj.isEmissive;

		sceneObjects_.push_back(std::move(sceneObject));

		sceneObjectData_.emplace_back(std::make_unique<SceneObjectData>(obj));
	}
}

void SceneObjectSystem::SceneObjectManager::ResetObjects()
{
	sceneObjects_.clear();

	const auto& assetManager = AssetManager::GetInstance();

	stageObjects_.SetModel(assetManager->modelMap.Get("Stage")->Get());
	stageObjects_.SetWorldMatrix(Matrix4x4::identity);
	for (const auto& obj : sceneObjectData_) {

		auto sceneObject = std::make_unique<SceneObject>();

		sceneObject->model_.SetModel(assetManager->modelMap.Get(obj->modelName)->Get());

		//blender->左手座標系
		sceneObject->transform = obj->transform;
		sceneObject->transform.scale = SceneObjectConverter::ConvertSizeToLeftHand(sceneObject->transform.scale);
		sceneObject->transform.rotate = SceneObjectConverter::ConvertRotateToLeftHand(sceneObject->transform.rotate);
		sceneObject->transform.translate = SceneObjectConverter::ConvertTranslateToLeftHand(sceneObject->transform.translate);
		sceneObject->transform.UpdateMatrix();


		if (obj->capsuleCollisionData) {
			sceneObject->collider = std::make_shared<CapsuleCollider>(
				CollisionCategory::LIGHT,
				CollisionCategory::FLASHLIGHT,
				Vector3::zero,
				0.0f,
				0.0f,
				Quaternion::identity);
			sceneObject->collider->get()->center = SceneObjectConverter::ConvertTranslateToLeftHand(obj->capsuleCollisionData->center);
			sceneObject->collider->get()->quaternion = SceneObjectConverter::ConvertRotateToLeftHand(obj->capsuleCollisionData->quaternion);
			sceneObject->collider->get()->radius = obj->capsuleCollisionData->radius;
			sceneObject->collider->get()->height = obj->capsuleCollisionData->height;
		}

		sceneObject->isEmissive = obj->isEmissive;

		sceneObjects_.push_back(std::move(sceneObject));
	}

}

void SceneObjectSystem::SceneObjectManager::Update()
{
	for (const auto& obj : sceneObjects_) {
		obj->transform.UpdateMatrix();
		obj->model_.SetWorldMatrix(obj->transform.worldMatrix);
		if (obj->isEmissive &&
			!obj->collider->get()->GetCollidedWith().empty()) {
			obj->isEmissive = false;
			obj->collider = std::nullopt;
		}
	}
}
