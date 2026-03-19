#pragma once
#include "Math/Transform.h"
#include "Graphics/Sprite.h"
#include <memory>
#include <array>
#include <random>
class SpriteParticles {
public:

    static const uint32_t kMaxParticleNum_ = 120;

    struct Particle {
        Sprite sprite;
        Vector2 position;
        Vector2 startPosition;
        int frame = 0;
        int maxFrame = 60;
        bool isActive = false;
    };

    void Initialize();
    void Update();
    void Emit(int count = 1);

    void SetEmitCenter(const Vector2& center) { emitCenter_ = center; }
    void SetEmitRadius(float radius) { emitRadius_ = radius; }
    void SetTargetPosition(const Vector2& target) { targetPosition_ = target; }
    void SetAutoEmit(bool enable, int intervalFrames = 3);
    void SetColor(const Color& color) { color_ = color; }
private:
    void InitParticle(Particle& p);
    Vector2 GetRandomPositionInCircle();

    std::array<Particle, kMaxParticleNum_> particle_;

    // エミッター設定
    Vector2 emitCenter_ = { 640.0f, 120.0f };
    float emitRadius_ = 100.0f;
    Vector2 targetPosition_ = { 1090.0f, 80.0f };

    // 自動生成
    bool autoEmit_ = true;
    int emitInterval_ = 3;
    int emitTimer_ = 0;

    // 乱数
    std::mt19937 rng_;

    Color color_;
 
};
