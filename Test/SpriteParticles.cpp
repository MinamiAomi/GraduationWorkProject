#include "SpriteParticles.h"
#include "Math/Color.h"
#include "Framework/AssetManager.h"
#include "Scene/SceneManager.h"
#include "GameScene.h"
#include "Trolley.h"
void SpriteParticles::Initialize()
{

    // 乱数初期化
    std::random_device rd;
    rng_ = std::mt19937(rd());

    auto assetManager = AssetManager::GetInstance();
    auto spriteImg = assetManager->textureMap.Get("effect")->Get();

    for (auto& p : particle_) {
        p.sprite.SetTexture(spriteImg);
        p.sprite.SetPosition({ 640.0f, 120.0f });
        p.sprite.SetAnchor({ 0.5f, 0.5f });
        p.sprite.SetScale({ 16.0f, 16.0f });
        p.sprite.SetUVRect({ {0.0f, 0.0f}, {1.0f, 1.0f} }, Sprite::UVMode::UV);
        p.sprite.SetDrawOrder(5);
        p.sprite.SetIsActive(false);  // 最初は非表示
        p.isActive = false;
    }
    emitTimer_ = 0;
}
void SpriteParticles::Update()
{

#ifdef _DEBUG
    ImGui::Begin("Particle Debug");
    ImGui::DragFloat2("Emit Center", &emitCenter_.x);
    ImGui::DragFloat("Emit Radius", &emitRadius_, 1.0f, 0.0f, 200.0f);
    ImGui::DragFloat2("Target Position", &targetPosition_.x);
    ImGui::DragInt("Emit Interval", &emitInterval_, 1, 1, 60);
    if (ImGui::Button("Emit 10")) {
        Emit(10);
    }
    int activeCount = 0;
    for (const auto& p : particle_) {
        if (p.isActive) activeCount++;
    }
    ImGui::Text("Active Particles: %d", activeCount);
    ImGui::End();
#endif

    // GameScene以外では全て非表示
    if (!dynamic_cast<GameScene*>(SceneManager::GetInstance()->GetCurrentScene())) {
        for (auto& p : particle_) {
            p.sprite.SetIsActive(false);
        }
        return;
    }

    const auto& state = Trolley::GetInstance()->GetState();

    if (Trolley::GetInstance()->GetIsHitFlashlight() && !Trolley::GetInstance()->GetFlashlight()->GetBatteryRemaining()) {
        if (state == Trolley::State::Normal) {
            SetAutoEmit(true, 3);
            SetColor(Color{ 255.0f / 255.0f,206.0f / 255.0f,0.0f / 255.0f,1.0f });
        }else if(state == Trolley::State::Overcharge){
            SetAutoEmit(true, 3);
            SetColor(Color{ 0.0f / 255.0f,190.0f / 255.0f,217.0f / 255.0f,1.0f });
        }
    }
    else {
        SetAutoEmit(false, 3);
    }

    // 自動生成
    if (autoEmit_) {
        emitTimer_++;
        if (emitTimer_ >= emitInterval_) {
            Emit(1);
            emitTimer_ = 0;
        }
    }

    // 各パーティクルを更新
    for (auto& p : particle_) {
        if (!p.isActive) continue;

        p.frame++;
        float t = static_cast<float>(p.frame) / static_cast<float>(p.maxFrame);

        // 寿命切れ
        if (t >= 1.0f) {
            p.isActive = false;
            p.sprite.SetIsActive(false);
            continue;
        }

        // 位置更新（開始位置から目標へ線形補間）
        p.position.x = p.startPosition.x + (targetPosition_.x - p.startPosition.x) * t;
        p.position.y = p.startPosition.y + (targetPosition_.y - p.startPosition.y) * t;
        p.sprite.SetPosition(p.position);

        // フェードアウト（70%以降）
        float alpha = 1.0f;
        if (t > 0.7f) {
            alpha = 1.0f - (t - 0.7f) / 0.3f;
        }
        Color original = p.sprite.GetColor();
        p.sprite.SetColor({ original.GetR(), original.GetG(), original.GetB(), alpha});

        // スケール変化（徐々に小さく）
        float scale = 16.0f * (1.0f - t * 0.5f);
        p.sprite.SetScale({ scale, scale });
    }
}

void SpriteParticles::Emit(int count)
{
    for (int i = 0; i < count; ++i) {
        for (auto& p : particle_) {
            if (!p.isActive) {
                InitParticle(p);
                break;
            }
        }
    }
}

void SpriteParticles::InitParticle(Particle& p)
{
    p.isActive = true;
    p.sprite.SetIsActive(true);
    p.sprite.SetColor(color_);

    // 円内のランダムな位置から開始
    p.startPosition = GetRandomPositionInCircle();
    p.position = p.startPosition;
    p.sprite.SetPosition(p.position);

    // 寿命をランダムに設定（48〜90フレーム）
    std::uniform_int_distribution<int> frameDist(48, 90);
    p.maxFrame = frameDist(rng_);
    p.frame = 0;

    // 初期状態にリセット
    p.sprite.SetScale({ 16.0f, 16.0f });
}

Vector2 SpriteParticles::GetRandomPositionInCircle()
{
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
    std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);

    float angle = angleDist(rng_);
    // sqrtで均一な分布にする
    float r = emitRadius_ * std::sqrt(radiusDist(rng_));

    return {
        emitCenter_.x + r * std::cosf(angle),
        emitCenter_.y + r * std::sinf(angle)
    };
}

void SpriteParticles::SetAutoEmit(bool enable, int intervalFrames)
{
    autoEmit_ = enable;
    emitInterval_ = intervalFrames;
}