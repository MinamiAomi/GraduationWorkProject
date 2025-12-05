#include "TitleScene.h"

#include "PersistentData.h"
#include "StageSelectScene.h"
#include "Graphics/RenderManager.h"
#include "Engine/Framework/AssetManager.h"
#include "Engine/File/JsonConverter.h"

void TitleScene::OnInitialize() {
	persistentData_ = SceneManager::GetInstance()->GetPersistentData();
	if (persistentData_) {
		
	}
	input_ = Input::GetInstance();
	camera_ = std::make_shared<Camera>();
	camera_->UpdateMatrices();

	RenderManager::GetInstance()->SetCamera(camera_);

	
	
	auto assetManager = AssetManager::GetInstance();
	
	JSON_OPEN("Resources/Data/Title/title.json");
	JSON_OBJECT("title");
	for (int i = 0; i < stoneNum; i++) {
		stoneModels_[i] = std::make_unique<ModelInstance>();
		stoneTransforms_[i] = std::make_unique<Transform>();
		stoneTransforms_[i]->rotate = Quaternion::MakeForXAxis(-90.0f * Math::ToRadian);
		stoneTransforms_[i]->scale = Vector3{ 0.8f,0.8f,0.8f };
		stoneTransforms_[i]->UpdateMatrix();
		stoneModels_[i]->SetWorldMatrix(stoneTransforms_[0]->worldMatrix);

		std::string name = "stonePositions" + std::to_string(i);
		JSON_LOAD_BY_NAME(name,stonePositions_[i]);
	}
	stoneModels_[0]->SetModel(assetManager->modelMap.Get("O")->Get());
	stoneModels_[1]->SetModel(assetManager->modelMap.Get("r")->Get());
	stoneModels_[2]->SetModel(assetManager->modelMap.Get("e")->Get());
	stoneModels_[3]->SetModel(assetManager->modelMap.Get("n")->Get());
	stoneModels_[4]->SetModel(assetManager->modelMap.Get("o")->Get());
	stoneModels_[5]->SetModel(assetManager->modelMap.Get("T")->Get());
	stoneModels_[6]->SetModel(assetManager->modelMap.Get("o")->Get());
	stoneModels_[7]->SetModel(assetManager->modelMap.Get("u")->Get());
	stoneModels_[8]->SetModel(assetManager->modelMap.Get("s")->Get());
	stoneModels_[9]->SetModel(assetManager->modelMap.Get("o")->Get());
	stoneModels_[10]->SetModel(assetManager->modelMap.Get("u")->Get());
	stoneModels_[11]->SetModel(assetManager->modelMap.Get("G")->Get());
	stoneModels_[12]->SetModel(assetManager->modelMap.Get("e")->Get());
	stoneModels_[13]->SetModel(assetManager->modelMap.Get("k")->Get());
	stoneModels_[14]->SetModel(assetManager->modelMap.Get("i")->Get());

	JSON_CLOSE();
}

void TitleScene::OnUpdate() {
#ifdef _DEBUG
	ImGui::Begin("TitleScene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::TreeNode("Title")) {
		if (ImGui::TreeNode("Stone")) {
			for (int i = 0; i < stoneNum; i++) {
				std::string name = "stonePositions" + std::to_string(i);
				ImGui::DragFloat3(name.c_str(), &stonePositions_[i].x,0.01f);
			}

			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Title/title.json");
				JSON_OBJECT("title");
				for (int i = 0; i < stoneNum; i++) {
					std::string name = "stonePositions" + std::to_string(i);
					JSON_SAVE_BY_NAME(name, stonePositions_[i]);
				}
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	ImGui::End();

	for (int i = 0; i < stoneNum; i++) {
		stoneTransforms_[i]->translate = stonePositions_[i];
		stoneTransforms_[i]->rotate = Quaternion::MakeForXAxis(-90.0f * Math::ToRadian);
		stoneTransforms_[i]->UpdateMatrix();
		stoneModels_[i]->SetWorldMatrix(stoneTransforms_[i]->worldMatrix);
	}

#endif // _DEBUG
	if (input_->IsKeyTrigger(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene<StageSelectScene>();
	}
}

void TitleScene::OnFinalize() {

}
