// A-Trous Filter

#define NUM_THREADS 32

Texture2D<float32_t4> g_SourceBuffer : register(t0);
RWTexture2D<float32_t4> g_DenoisedBuffer : register(u0);

Texture2D<float32_t4> g_Albedo : register(t0, space1);
Texture2D<float32_t4> g_MetallicRoughness : register(t1, space1);
Texture2D<float32_t4> g_Normal : register(t2, space1);
Texture2D<float32_t4> g_Depth : register(t3, space1);

struct Common {
    float32_t albedoSigma;
    float32_t normalSigma;
    float32_t depthSigma;
};
ConstantBuffer<Common> g_Common : register(b0);

float32_t KernelWeight(in int32_t2 offset) {
    const float32_t kernels[] = { 1.0f / 16.0f, 1.0f / 4.0f, 3.0f / 8.0f, 1.0f / 4.0f, 1.0f / 16.0f };
    return kernels[offset.x] * kernels[offset.y];
}

float32_t GeometryWeight(in float32_t3 delta, in float32_t sigma) {
    float32_t dist2 = dot(delta, delta);
    return min(exp(-dist2 / sigma), 1.0f);
}

// 5 * 5 Filter
float32_t3 ATrousFilter(in int32_t2 center, in int32_t2 textureSize) {
    float32_t3 color = float32_t3(0.0f, 0.0f, 0.0f);
    float32_t weight = 0.0f;
    int32_t2 sampleLimits = textureSize - 1;

    float32_t4 centerAlbedo = g_Albedo[center];

    if (centerAlbedo.a == 0.0f) {
        return g_SourceBuffer[center].rgb;
    }

    float32_t2 centerMetallicRoughness = g_MetallicRoughness[center].rg;
    float32_t3 centerNormal = g_Normal[center].xyz * 2.0f - 1.0f;
    float32_t centerDepth = g_Depth[center].x;

    for (int32_t y = 0; y < 5; ++y) {
        for (int32_t x = 0; x < 5; ++x) {

            //int32_t offset = 
            int32_t2 samplePixel = clamp(center + int32_t2(x - 2, y - 2), int32_t2(0, 0), sampleLimits);

            float32_t4 albedo = g_Albedo[samplePixel];
            if (albedo.a == 0.0f) { continue; }
            float32_t2 metallicRoughness = g_MetallicRoughness[samplePixel].rg;
            float32_t3 normal = g_Normal[samplePixel].xyz * 2.0f - 1.0f;
            float32_t depth = g_Depth[samplePixel].x;

            if (metallicRoughness.x != centerMetallicRoughness.x || metallicRoughness.y != centerMetallicRoughness.y) { continue; }

            float32_t albedoWeight = GeometryWeight(albedo.rgb - centerAlbedo.rgb, g_Common.albedoSigma);
            float32_t normalWeight = GeometryWeight(normal - centerNormal, g_Common.normalSigma);
            float32_t depthWeight = min(exp(-abs(depth - centerDepth) / g_Common.depthSigma), 1.0f);

            float32_t geometryWeight = albedoWeight * normalWeight * depthWeight;
            float32_t kernelWeight = KernelWeight(int32_t2(x, y));
            float32_t w = geometryWeight * kernelWeight;
            color += g_SourceBuffer[samplePixel].rgb * w;
            weight += w;
        }
    }
    return color / weight;
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID, uint32_t3 GTid : SV_GroupThreadID) {

    int32_t2 samplePixel = (int32_t2)DTid.xy;
    float32_t2 textureSize;
    g_SourceBuffer.GetDimensions(textureSize.x, textureSize.y);

    // 範囲外の場合何もしない
    if (samplePixel.x >= textureSize.x || samplePixel.y >= textureSize.y) { return; }


    float32_t4 color = float32_t4(0.0f, 0.0f, 0.0f, 1.0f);
    // フィルター
    color.rgb = ATrousFilter(samplePixel, textureSize);

    g_DenoisedBuffer[samplePixel] = color;
}