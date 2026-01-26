#pragma once

#include <memory>
#include <list>
#include <vector>

#include "Bats.h"


class Camera;
class BatsManager {
public:
	void Initialize();
	void Update();


	void Emit(const std::vector<std::vector<bool>>&  emit);
	void SetCamera(const Camera* camera) { camera_ = camera; }
private:
	const Camera* camera_;
	std::list<std::unique_ptr<Bats>> batsManager_;
};