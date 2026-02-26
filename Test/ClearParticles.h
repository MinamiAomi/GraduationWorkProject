#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"

#include "ParticleDefine.h"

class ClearParticles
{
public:

#ifdef _DEBUG
	static void Debug();
#endif // _DEBUG

	void Initialize();
	void Update();
	void Emit();
	void DebugDraw();

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(const Transform* parent) { transform_.SetParent(parent, false); }
public:
	inline static int emitNum_ = 1;
	inline static int deadFrame_ = 300;
	inline static Vector3 startScale_ = {0.1f,0.1f,0.1f};
	inline static float gravity_ = -0.001f;
	inline static Vector3 size_ = {1.0f,1.0f,1.0f};
	inline static Vector3 colorR_ = { 1.0f,0.5f,0.5f };
	inline static Vector3 colorG_ = { 0.5f,1.0f,0.5f };
	inline static Vector3 colorB_ = { 0.5f,0.5f,1.0f };
	inline static Vector3 position_ = Vector3::zero;
	inline static Vector3 minAngularVelocity_ = { -0.05f, -0.05f, -0.05f };
	inline static Vector3 maxAngularVelocity_ = { 0.05f,  0.05f,  0.05f };
	
private:
	struct Particle {
		ModelInstance modelInstance_;
		Transform transform_;
		Vector3 angularVelocity_;
		Vector3 velocity_;
		std::shared_ptr<Material> material_;
		int deadFrame_;
	};


	Transform transform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Particle>> particles_;
	Random::RandomNumberGenerator rnd_;
};

