#include "Pathtracing.hlsli"

// texcoodとdepthからワールド座標を計算
float32_t3 GetWorldPosition(in float32_t2 texcoord, in float32_t depth, in float32_t4x4 viewProjectionInverseMatrix) {
    // xは0~1から-1~1, yは0~1から1~-1に上下反転
    float32_t2 xy = texcoord * float32_t2(2.0f, -2.0f) + float32_t2(-1.0f, 1.0f);
    float32_t4 position = float32_t4(xy, depth, 1.0f);
    position = mul(position, viewProjectionInverseMatrix);
    position.xyz /= position.w;
    return position.xyz;
}

[shader("raygeneration")]
void StandardRGS() {
    // レイのインデックス
    uint32_t2 dispatchRaysIndex = DispatchRaysIndex().xy;
    // レイの縦横数
    uint32_t2 dispatchRaysDimensions = DispatchRaysDimensions().xy;
    // テクスチャ座標系を求める    
    float32_t2 texcoord = ((float32_t2)dispatchRaysIndex + 0.5f) / (float32_t2)dispatchRaysDimensions;

    // 近面から遠面へのレイ
    RayDesc ray;
    float32_t3 rayOrigin = GetWorldPosition(texcoord, 0.0f, g_Scene.viewProjectionInverseMatrix);
    ray.Origin = rayOrigin;
    float32_t3 rayDiff = GetWorldPosition(texcoord, 1.0f, g_Scene.viewProjectionInverseMatrix) - rayOrigin;
    ray.Direction = normalize(rayDiff);
    ray.TMin = 0.0f;
    ray.TMax = length(rayDiff);
    // レイを飛ばす
    Payload payload;
    payload.color = float32_t3(0.0f, 0.0f, 0.0f);
    payload.recursiveCount = 0; // 再帰回数 0
    payload.seed = Srand(int32_t2(dispatchRaysIndex), g_Scene.frame);
    TraceRay(
        g_TLAS, // RaytracingAccelerationStructure
        // 背面カリング
        RAY_FLAG_NONE, // RayFlags
        // レイのマスク
        VISIBILITY_MASK, // InstanceInclusionMask
        // シェーダーテーブルのオフセット
        RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
        // シェーダーテーブルの係数
        MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
        // ミスシェーダーのシェーダーテーブル
        MISS_SHADER_INDEX, // MissShaderIndex
        ray, // Ray
        payload // Payload
    );

    g_Color[dispatchRaysIndex].rgb = payload.color;
    g_Color[dispatchRaysIndex].a = 1.0f;
}
