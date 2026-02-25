#include "IcicleBreak.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/File/JsonConverter.h"

void IcicleBreak::Initialize()
{
	
}

void IcicleBreak::Update()
{
	for (auto it = modelInfo_.begin(); it != modelInfo_.end(); ) {

		ModelInfo* p = it->get();

		p->animationTime += 0.016f / p->duration_;
	
		const AnimationSet& anime0 = p->animation[0]->Get()->GetAnimation("IcicleAction");
		auto node = anime0.nodeAnimations.find("Icicle0");

		if (node != anime0.nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = node->second;
			p->transform_[0].translate = CalculateValue(nodeAnim.translate, p->animationTime);
			p->transform_[0].rotate = CalculateValue(nodeAnim.rotate, p->animationTime);
			p->transform_[0].scale = CalculateValue(nodeAnim.scale, p->animationTime);
		}

		const AnimationSet& anime1 = p->animation[1]->Get()->GetAnimation("Icicle.003Action");
		node = anime1.nodeAnimations.find("Icicle1");

		if (node != anime1.nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = node->second;
			p->transform_[1].translate = CalculateValue(nodeAnim.translate, p->animationTime);
			p->transform_[1].rotate = CalculateValue(nodeAnim.rotate, p->animationTime);
			p->transform_[1].scale = CalculateValue(nodeAnim.scale, p->animationTime);
		}

		const AnimationSet& anime2 = p->animation[2]->Get()->GetAnimation("Icicle.002Action");
		node = anime2.nodeAnimations.find("Icicle2");

		if (node != anime2.nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = node->second;
			p->transform_[2].translate = CalculateValue(nodeAnim.translate, p->animationTime);
			p->transform_[2].rotate = CalculateValue(nodeAnim.rotate, p->animationTime);
			p->transform_[2].scale = CalculateValue(nodeAnim.scale, p->animationTime);
		}

		const AnimationSet& anime3 = p->animation[3]->Get()->GetAnimation("Icicle.001Action");
		node = anime3.nodeAnimations.find("Icicle3");

		if (node != anime3.nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = node->second;
			p->transform_[3].translate = CalculateValue(nodeAnim.translate, p->animationTime);
			p->transform_[3].rotate = CalculateValue(nodeAnim.rotate, p->animationTime);
			p->transform_[3].scale = CalculateValue(nodeAnim.scale, p->animationTime);
		}
		
		if (p->animationTime >= 1.0f) {
			it = modelInfo_.erase(it);
		}
		else {
			// 行列更新
			for (int i = 0; i < 4; i++) {
				p->transform_[i].UpdateMatrix();
				p->modelInstance_[i].SetWorldMatrix(p->transform_[i].worldMatrix);
			}

			++it;
		}
	}
}

void IcicleBreak::Emit(const Transform& copyTransform)
{
	auto assetManager = AssetManager::GetInstance();
	auto newTomb = std::make_unique<ModelInfo>();

	newTomb->modelInstance_[0].SetModel(assetManager->modelMap.Get("IcicleBreak0")->Get());
	newTomb->modelInstance_[1].SetModel(assetManager->modelMap.Get("IcicleBreak1")->Get());
	newTomb->modelInstance_[2].SetModel(assetManager->modelMap.Get("IcicleBreak2")->Get());
	newTomb->modelInstance_[3].SetModel(assetManager->modelMap.Get("IcicleBreak3")->Get());
	newTomb->parentTransform_ = copyTransform;
	newTomb->parentTransform_.rotate = Quaternion::identity;
	newTomb->parentTransform_.UpdateMatrix();
	newTomb->transform_[0].SetParent(&newTomb->parentTransform_, false);
	newTomb->transform_[1].SetParent(&newTomb->parentTransform_, false);
	newTomb->transform_[2].SetParent(&newTomb->parentTransform_, false);
	newTomb->transform_[3].SetParent(&newTomb->parentTransform_, false);


	newTomb->animation[0] = assetManager->animationMap.Get("IcicleBreak0Anim");
	newTomb->animation[1] = assetManager->animationMap.Get("IcicleBreak1Anim");
	newTomb->animation[2] = assetManager->animationMap.Get("IcicleBreak2Anim");
	newTomb->animation[3] = assetManager->animationMap.Get("IcicleBreak3Anim");

	const AnimationSet& anime = newTomb->animation[0]->Get()->GetAnimation("IcicleAction");
	auto it = anime.nodeAnimations.find("Icicle0");

	newTomb->animationTime = 0.0f;
	newTomb->duration_ = anime.duration;

	modelInfo_.push_back(std::move(newTomb));
	
}
