#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"

#include "ParticleDefine.h"

class IcicleParticle
{
public:

#ifdef _DEBUG
	static void Debug();
#endif // _DEBUG

	void Initialize();
	void Update();
	void Emit(const Vector3& position);
	void Emit();

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(const Transform* parent) { transform_.SetParent(parent, false); }
public:
	//std::shared_ptr<Material> material_;
	inline static float minSpeed_ = 0.002f;
	inline static float maxSpeed_ = 0.003f;
	inline static Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	inline static Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	inline static float scaleDecay_ = 0.02f;
	inline static int emitNum_ = 20;
	inline static float startScale_ = 0.1f;
	inline static float gravity_ = -0.001f;
	inline static Vector3 color_ = { 0.2f,0.2f,1.0f };
	
private:
	struct Particle {
		ModelInstance modelInstance_;
		Transform transform_;
		Vector3 angularVelocity_;
		Vector3 velocity_;
		float scaleSpeed_;
	};


	Transform transform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Random::RandomNumberGenerator rnd_;
	EmitShape emitShapeType_;
};

