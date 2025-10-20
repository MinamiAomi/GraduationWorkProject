#pragma once
#include <memory>
#include <filesystem>
#include <list>
#include <vector>

#include "Texture.h"

#include "Math/MathUtils.h"

class Sprite {
    friend class SpriteRenderer;
public:
    Sprite();
    ~Sprite();
    
    // セッター

    void SetTexture(const std::shared_ptr<Texture>& texture) { texture_ = texture; }
    void SetPosition(const Vector2& position) { position_ = position; }
    void SetRotate(float rotate) { rotate_ = rotate; }
    void SetScale(const Vector2& scale) { scale_ = scale; }
    void SetAnchor(const Vector2& anchor) { anchor_ = anchor; }
    void SetTexcoordRect(const Vector2& base, const Vector2& size) { texcoordBase_ = base, texcoordSize_ = size; }
    void SetColor(const Vector4& color) { color_ = color; }
    void SetDrawOrder(uint8_t drawOrder) { drawOrder_ = drawOrder; }
    void SetIsActive(bool isActive) { isActive_ = isActive; }

    // ゲッター

    uint8_t GetDrawOrder() const { return drawOrder_; }

private:
    static std::list<Sprite*> instanceList_;

    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
    Sprite(Sprite&&) = delete;
    Sprite& operator=(Sprite&&) = delete;

    std::shared_ptr<Texture> texture_;
    Vector2 position_;
    float rotate_ = 0.0f;
    Vector2 scale_ = Vector2::one;
    Vector2 anchor_ = { 0.5f,0.5f };
    Vector2 texcoordBase_ = {  0.0f,  0.0f };
    Vector2 texcoordSize_ = { 64.0f, 64.0f };
    Vector4 color_ = Vector4::one;
    uint8_t drawOrder_ = 0; // 小さいほど上に描画される レイヤーを表す値
    bool isActive_ = true;
};