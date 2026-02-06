#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"
#include "ParticleDefine.h"

class Camera;

class BatsBullets
{
public:

	void Initialize(const Camera& camera);
	void Update();
	void SetBatTransform(const Transform& transform) {
		parentTransform_ = &transform;
	}
	bool HasParticles() const {
		return !particles_.empty();
	}
	void SetIsEmit(bool isEmit) { isEmit_ = isEmit; }
public:
	inline static int emitInterval_;
	std::shared_ptr<Material> material_;
	inline static float speed_;
private:
	void Emit(int num);
private:
	struct Particle {
		ModelInstance modelInstance_;
		Transform transform_;
	};

	bool isEmit_ = false;

	const Camera* camera_ = nullptr;

	const Transform* parentTransform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;

	EmitShape emitShapeType_; 
	int emitTimer_ = 0;
};
