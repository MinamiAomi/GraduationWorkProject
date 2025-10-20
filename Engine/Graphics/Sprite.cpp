#include "Sprite.h"

#include "Core/TextureLoader.h"
#include "Core/SamplerManager.h"

std::list<Sprite*> Sprite::instanceList_;

Sprite::Sprite() {
	instanceList_.emplace_back(this);
}

Sprite::~Sprite() {
	instanceList_.remove(this);
}
