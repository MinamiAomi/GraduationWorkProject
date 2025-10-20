///
/// 基本クラスからjson構造体に変換
/// 

#pragma once

#include "Externals/nlohmann/json.hpp"

#include <Math/MathUtils.h>
#include <Math/Transform.h>

void to_json(nlohmann::json& json, const Vector2& value);
void to_json(nlohmann::json& json, const Vector3& value);
void to_json(nlohmann::json& json, const Vector4& value);
void to_json(nlohmann::json& json, const Quaternion& value);
void to_json(nlohmann::json& json, const Transform& value);

void from_json(const nlohmann::json& json, Vector2& value);
void from_json(const nlohmann::json& json, Vector3& value);
void from_json(const nlohmann::json& json, Vector4& value);
void from_json(const nlohmann::json& json, Quaternion& value);
void from_json(const nlohmann::json& json, Transform& value);