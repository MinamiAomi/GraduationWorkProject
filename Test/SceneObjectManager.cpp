#include "SceneObjectManager.h"

#include "RailConverter.h"
#include "SceneObjectConverter.h"

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
		if (!obj->collider->GetCollidedWith().empty()) {
			obj->collider = nullptr;
			obj->lightObject.SetHp(0.0f);
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
}

void SceneObjectSystem::SceneObjectManager::BuildRuntimeObjects()
{
	for (const auto& objDataPtr : sceneObjectData_) {
		const auto& data = *objDataPtr;

		switch (data.type)
		{
		case SceneObjectSystem::ObjectType::PointLight:
		{
			auto pointLightObject = std::make_unique<PointLightObject>();

			InitializeCommonObject(pointLightObject, data);

			if (data.pointLightData) {
				pointLightObject->lightObject.Initialize(&pointLightObject->transform);

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
			InitializeCommonObject(emitterObject, data);

			emitterObjects_.push_back(std::move(emitterObject));
		}
		break;

		case SceneObjectSystem::ObjectType::Enemy:
		{
			auto enemyObject = std::make_unique<EnemyObject>();
			InitializeCommonObject(enemyObject, data);
			
			enemyObjects_.push_back(std::move(enemyObject));
		}
		break;

		default:
			break;
		}
	}
}
