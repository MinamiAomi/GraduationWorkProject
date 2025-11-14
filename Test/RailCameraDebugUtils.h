#pragma once

#include <array>

#include "Math/MathUtils.h"

namespace RailCameraSystem {

    class RailCameraDebugUtils {
    public:
        static std::array<Vector3,8> CalculateFrustum(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
            //View-Projection行列とその逆行列を計算
            const Matrix4x4 viewProjMatrix = viewMatrix * projectionMatrix;
            const Matrix4x4 invViewProjMatrix = viewProjMatrix.Inverse();

            //正規化デバイス座標(NDC)空間における視錐台の8頂点を定義します。
            const std::array<Vector4, 8> ndcCorners = {
                // Near Plane
                Vector4{-1.0f, -1.0f, 0.0f, 1.0f}, // 左下
                Vector4{ 1.0f, -1.0f, 0.0f, 1.0f}, // 右下
                Vector4{ 1.0f,  1.0f, 0.0f, 1.0f}, // 右上
                Vector4{-1.0f,  1.0f, 0.0f, 1.0f}, // 左上
                // Far Plane
                Vector4{-1.0f, -1.0f, 1.0f, 1.0f}, // 左下
                Vector4{ 1.0f, -1.0f, 1.0f, 1.0f}, // 右下
                Vector4{ 1.0f,  1.0f, 1.0f, 1.0f}, // 右上
                Vector4{-1.0f,  1.0f, 1.0f, 1.0f}  // 左上
            };

            std::array<Vector3, 8> worldFrustum;
            for (int i = 0; i < 8; ++i) {
                //NDC座標を逆View-Projection行列で変換し、ワールド座標に戻す
                Vector4 worldCorner =  ndcCorners[i]* invViewProjMatrix;

                //3D座標に変換
                if (worldCorner.w != 0.0f) {
                    worldFrustum[i].x = worldCorner.x / worldCorner.w;
                    worldFrustum[i].y = worldCorner.y / worldCorner.w;
                    worldFrustum[i].z = worldCorner.z / worldCorner.w;
                }
            }

            return worldFrustum;
        }
    };

}