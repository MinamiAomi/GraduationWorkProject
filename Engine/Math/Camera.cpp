#include "Camera.h"

Camera::Camera(ProjectionType projectionType) {
    projectionType_ = projectionType;
    transform_.translate = { 0.0f, 1.0f, -6.0f };
    transform_.rotate = Quaternion::MakeLookRotation(-transform_.translate);

    projection_.perspective.fovY = 45.0f * Math::ToRadian;
    projection_.perspective.aspectRaito = 1280.0f / 720.0f;
    nearClip_ = 0.1f;
    farClip_ = 500.0f;

    needUpdateing_ = true;

    UpdateMatrices();
}

void Camera::UpdateMatrices() {
    transform_.UpdateMatrix();
    if (needUpdateing_) {
        viewMatrix_ = Matrix4x4::MakeAffineInverse(Matrix4x4::MakeRotation(transform_.rotate), transform_.translate);

        switch (projectionType_)
        {
        default:
        case Camera::Perspective:
            projectionMatrix_ = Matrix4x4::MakePerspectiveProjection(projection_.perspective.fovY, projection_.perspective.aspectRaito, nearClip_, farClip_);
            break;
        case Camera::Orthographic:
            projectionMatrix_ = Matrix4x4::MakeOrthographicProjection(projection_.orthographic.width, projection_.orthographic.height, nearClip_, farClip_);
            break;
        }

        viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;
        viewProjectionInverseMatrix_ = viewProjectionMatrix_.Inverse();
    }                
    frustum_.Set(viewProjectionMatrix_);
}

void Camera::SetPerspective(float fovY, float aspectRaito, float nearClip, float farClip) {
    projection_.perspective.fovY = fovY;
    projection_.perspective.aspectRaito = aspectRaito;
    nearClip_ = nearClip;
    farClip_ = farClip;
    projectionType_ = Perspective;
    needUpdateing_ = true;
}

void Camera::SetOrthographic(float width, float height, float nearClip, float farClip) {
    projection_.orthographic.width = width;
    projection_.orthographic.height = height;
    nearClip_ = nearClip;
    farClip_ = farClip;
    projectionType_ = Orthographic;
    needUpdateing_ = true;
}

std::array<Vector3, 8> Camera::GetFrustumVector3() const
{
    //View-Projection行列とその逆行列を計算
    const Matrix4x4 invViewProjMatrix = viewProjectionMatrix_.Inverse();

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
        Vector4 worldCorner = ndcCorners[i] * invViewProjMatrix;

        //3D座標に変換
        if (worldCorner.w != 0.0f) {
            worldFrustum[i].x = worldCorner.x / worldCorner.w;
            worldFrustum[i].y = worldCorner.y / worldCorner.w;
            worldFrustum[i].z = worldCorner.z / worldCorner.w;
        }
    }

    return worldFrustum;
}

std::array<Vector3, 8> Camera::GetFrustumVector3(float farClip) const
{
    //View-Projection行列とその逆行列を計算
    Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveProjection(projection_.perspective.fovY, projection_.perspective.aspectRaito, nearClip_, farClip);
    Matrix4x4 viewProjectionMatrix = Matrix4x4::MakeAffineInverse(Matrix4x4::MakeRotation(Quaternion{0.0f,0.0f,0.0f,1.0f}), transform_.translate) * projectionMatrix;

    const Matrix4x4 invViewProjMatrix = viewProjectionMatrix.Inverse();

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
        Vector4 worldCorner = ndcCorners[i] * invViewProjMatrix;

        //3D座標に変換
        if (worldCorner.w != 0.0f) {
            worldFrustum[i].x = worldCorner.x / worldCorner.w;
            worldFrustum[i].y = worldCorner.y / worldCorner.w;
            worldFrustum[i].z = worldCorner.z / worldCorner.w;
        }
    }

    return worldFrustum;
}

Vector3 Camera::GetFrustumGridCenter(float farClip, uint32_t splitX, uint32_t splitY, uint32_t indexX, uint32_t indexY) const
{
    std::array<Vector3, 8> worldFrustum = GetFrustumVector3(farClip);
    // worldFrustum の 4〜7 が Far Plane (左下, 右下, 右上, 左上)
    const Vector3& bottomLeft = worldFrustum[4];
    const Vector3& bottomRight = worldFrustum[5];
    const Vector3& topRight = worldFrustum[6];
    const Vector3& topLeft = worldFrustum[7];

    // グリッド内の中心位置を割合 (0.0 ～ 1.0) で計算
    // index + 0.5f とすることでマスの中心を指す
    float u = (static_cast<float>(indexX) + 0.5f) / static_cast<float>(splitX);
    float v = 1.0f - ((static_cast<float>(indexY) - 0.5f) / static_cast<float>(splitY));

    // 双線形補間 (Bilinear Interpolation) でワールド座標を算出
    // 下辺の補間
    Vector3 bottom = Vector3::Lerp(u,bottomLeft, bottomRight);
    // 上辺の補間
    Vector3 top = Vector3::Lerp(u,topLeft, topRight);

    // 上下辺の間を V 方向（高さ）で補間して中心点を返す
    return Vector3::Lerp(v,bottom, top);
}
