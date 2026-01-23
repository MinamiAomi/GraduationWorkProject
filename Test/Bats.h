#pragma once

#include <vector>
#include <memory>

#include "Math/MathUtils.h"
#include "Math/Transform.h"
#include "Math/Random.h"


#include "Graphics/Model.h"


class Bats
{
public:
	Bats(const std::vector<std::vector<bool>>& data);
	void Update();
	void DebugDraw();

	void SetRadius(float radius) { radius_ = radius; }

	void SetOffset(const Vector3& offset) { transform_.translate = offset; }
	void SetQuaternion(const Quaternion& offset) { transform_.rotate = offset; }
	void SetParent(Transform* parent) { transform_.SetParent(parent); }
public:
	std::shared_ptr<Material> material_;
private:
	void Emit(size_t r, size_t c);
private:
	struct Bat {
		ModelInstance modelInstance_;
		Transform transform_;
		Vector3 cameraToPos_;
		size_t rowIndex;
		size_t colIndex;
	};


	Transform transform_;
	std::shared_ptr<Model> model_;
	std::vector<std::unique_ptr<Bat>> bats_;
	Random::RandomNumberGenerator rnd_;
	//Sphere
	float radius_;
	
};

