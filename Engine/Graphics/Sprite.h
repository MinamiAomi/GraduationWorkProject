#pragma once
#include <memory>
#include <filesystem>
#include <list>
#include <vector>
#include <string>

#include "Texture.h"

#include "Math/MathUtils.h"
#include "Math/Color.h"

class Sprite {
	friend class SpriteRenderer;
public:
	enum class UVMode {
		UV,
		Texcoord
	};

	Sprite();
	~Sprite();

	// セッター

	void SetTexture(const std::shared_ptr<Texture>& texture) { texture_ = texture; }
	void SetPosition(const Vector2& position) { position_ = position; }
	void SetRotate(float rotate) { rotate_ = rotate; }
	void SetScale(const Vector2& scale) { scale_ = scale; }

	void SetAnchor(const Vector2& anchor) { anchor_ = anchor; }
	void SetUVRect(const Rect& rect, UVMode mode = UVMode::Texcoord) { uvRect_ = rect, uvMode_ = mode; }
	void SetColor(const Color& color) { color_ = color; }
	void SetDrawOrder(uint8_t drawOrder) { drawOrder_ = drawOrder; }
	void SetIsActive(bool isActive) { isActive_ = isActive; }
	void SetPre3DRender(bool pre3DRender) { pre3DRender_ = pre3DRender; }

	//俺が追加
	void SetIsCircleGauge(bool isCircleGauge) { isCircleGauge_ = isCircleGauge; }
	void SetCircleGaugeProgress(float circleGaugeProgress) { circleGaugeProgress_ = circleGaugeProgress; }
	// ゲッター

	uint8_t GetDrawOrder() const { return drawOrder_; }
	bool GetIsActive() const { return isActive_; }
	const Vector2& GetScale()const { return scale_; }
    bool GetPre3DRender() const { return pre3DRender_; }
	const Color& GetColor() const { return color_; }

	//俺が追加
	bool GetIsCircleGauge() const { return isCircleGauge_; }
	float GetCircleGaugeProgress()const { return circleGaugeProgress_; }
#ifdef _DEBUG
	void DrawImGui(const std::string& name);
#endif // _DEBUG
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
	Rect uvRect_;
	UVMode uvMode_ = UVMode::Texcoord;
	Color color_ = Color::white;
	uint8_t drawOrder_ = 0; // 小さいほど上に描画される レイヤーを表す値
	bool pre3DRender_ = false;
	bool isActive_ = true;

	//俺が追加提出2日前だからほんまごめん
	bool isCircleGauge_ = false;
	float circleGaugeProgress_ = 0.0f;
};