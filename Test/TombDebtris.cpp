#include "TombDebtris.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/File/JsonConverter.h"

void TombDebtris::Initialize()
{
	auto assetManager = AssetManager::GetInstance();
	spawnSESound_ = assetManager->soundMap.Get("SE_TOMB_BREAK")->Get();
}

void TombDebtris::Update()
{
	for (auto it = modelInfo_.begin(); it != modelInfo_.end(); ) {

		ModelInfo* p = it->get();

		p->animationTime += 0.016f / p->duration_;
	
		const AnimationSet& anime = p->animation[0]->Get()->GetAnimation("tombBreakLAction");
		auto node = anime.nodeAnimations.find("tombBreakL");

		if (node != anime.nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = node->second;
			p->transform_[0].translate = CalculateValue(nodeAnim.translate, p->animationTime);
			p->transform_[0].rotate = CalculateValue(nodeAnim.rotate, p->animationTime);
			p->transform_[0].scale = CalculateValue(nodeAnim.scale, p->animationTime);
		}

		const AnimationSet& animeR = p->animation[1]->Get()->GetAnimation("tombBreakRAction");
		node = animeR.nodeAnimations.find("tombBreakR");

		if (node != animeR.nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = node->second;
			p->transform_[1].translate = CalculateValue(nodeAnim.translate, p->animationTime);
			p->transform_[1].rotate = CalculateValue(nodeAnim.rotate, p->animationTime);
			p->transform_[1].scale = CalculateValue(nodeAnim.scale, p->animationTime);
		}
		
		if (p->animationTime >= 1.0f) {
			it = modelInfo_.erase(it);
		}
		else {
			// 行列更新
			p->transform_[0].UpdateMatrix();
			p->transform_[1].UpdateMatrix();
			p->modelInstance_[0].SetWorldMatrix(p->transform_[0].worldMatrix);
			p->modelInstance_[1].SetWorldMatrix(p->transform_[1].worldMatrix);

			++it;
		}
	}
}

void TombDebtris::Emit(const Transform& copyTransform)
{
	auto assetManager = AssetManager::GetInstance();
	auto newTomb = std::make_unique<ModelInfo>();

	newTomb->modelInstance_[0].SetModel(assetManager->modelMap.Get("TombBreakL")->Get());
	newTomb->modelInstance_[1].SetModel(assetManager->modelMap.Get("TombBreakR")->Get());
	newTomb->parentTransform_ = copyTransform;
	newTomb->transform_[0].SetParent(&newTomb->parentTransform_, false);
	newTomb->transform_[1].SetParent(&newTomb->parentTransform_, false);


	newTomb->animation[0] = assetManager->animationMap.Get("TombBreakLAnim");
	newTomb->animation[1] = assetManager->animationMap.Get("TombBreakRAnim");

	const AnimationSet& anime = newTomb->animation[0]->Get()->GetAnimation("tombBreakLAction");
	auto it = anime.nodeAnimations.find("tombBreakL");

	newTomb->animationTime = 0.0f;
	newTomb->duration_ = anime.duration;

	modelInfo_.push_back(std::move(newTomb));
	

	auto as = std::make_shared<AudioSource>();
	(*as) = spawnSESound_;
	as->SetVolume(0.7f);
	as->Play(false);
	playingAudioSourceList_.push_back(as);
}
