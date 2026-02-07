RWTexture2D<float32_t4> g_Texture : register(u0);

Texture2D<float32_t> g_Depth : register(t0);

struct Constant {
    float32_t4x4 viewProjectionInverseMatrix;

    float32_t3 cameraPosition;
    float32_t cameraNear;

    float32_t3 color;
    float32_t cameraFar;

    float32_t depthFogStart;
    float32_t depthFogEnd;
    float32_t hightFogStart;
    float32_t hightFogEnd;

    float32_t3 hightFogColor;
    float32_t fogFactor;
};
ConstantBuffer<Constant> g_Constant : register(b0);

float32_t3 GetWorldPosition(in float32_t2 texcoord, in float32_t depth) {
    // xは0~1から-1~1, yは0~1から1~-1に上下反転
    float32_t2 xy = texcoord * float32_t2(2.0f, -2.0f) + float32_t2(-1.0f, 1.0f);
    float32_t4 tmpPosition = float32_t4(xy, depth, 1.0f);
    tmpPosition = mul(tmpPosition, g_Constant.viewProjectionInverseMatrix);
    return tmpPosition.xyz / tmpPosition.w;
}

float32_t LinearizeDepth(float32_t depth) {
    return ((g_Constant.cameraNear * g_Constant.cameraFar) / (g_Constant.cameraFar - depth * (g_Constant.cameraFar - g_Constant.cameraNear)));
}

float Random(float2 uv, float seed) {
    return frac(sin(dot(uv, float2(12.9898f, 78.233f)) + seed) * 43758.5453f);
}

float2 Random2(float2 fact) {
    const float2 v1 = float2(127.1f, 311.7f);
    const float2 v2 = float2(269.5f, 183.3f);
    
    float2 angle = float2(dot(fact, v1), dot(fact, v2));
    return frac(sin(angle) * 43758.5453123f) * 2.0f - 1.0f;
}

float LerpRandom(float2 uv, float density, float seed) {
    float invDensity = 1.0f / density;
    float2 block = floor(uv * density);
    float2 pixel = frac(uv * density);
    //pixel = pixel * pixel * (3.0f - 2.0f * pixel);
    
    float v00 = Random((block + float2(0.0f, 0.0f)) * invDensity, 0.0f);
    float v01 = Random((block + float2(0.0f, 1.0f)) * invDensity, 0.0f);
    float v10 = Random((block + float2(1.0f, 0.0f)) * invDensity, 0.0f);
    float v11 = Random((block + float2(1.0f, 1.0f)) * invDensity, 0.0f);
    
    float v0010 = lerp(v00, v10, pixel.x);
    float v0111 = lerp(v01, v11, pixel.x);
    
    float random = lerp(v0010, v0111, pixel.y);
    return random;
}

float PerlinNoise(float2 uv, float density) {
    float2 uvFloor = floor(uv * density);
    float2 uvFrac = frac(uv * density);
    
    float2 v00 = Random2(uvFloor + float2(0.0f, 0.0f));
    float2 v01 = Random2(uvFloor + float2(0.0f, 1.0f));
    float2 v10 = Random2(uvFloor + float2(1.0f, 0.0f));
    float2 v11 = Random2(uvFloor + float2(1.0f, 1.0f));

    float c00 = dot(v00, uvFrac - float2(0.0f, 0.0f));
    float c01 = dot(v01, uvFrac - float2(0.0f, 1.0f));
    float c10 = dot(v10, uvFrac - float2(1.0f, 0.0f));
    float c11 = dot(v11, uvFrac - float2(1.0f, 1.0f));

    float2 u = uvFrac * uvFrac * (3.0f - 2.0f * uvFrac);
    
    float v0010 = lerp(c00, c10, u.x);
    float v0111 = lerp(c01, c11, u.x);

    return lerp(v0010, v0111, u.y) * 0.5f + 0.5f;
}

float FractalSumNoise(float2 uv, float density) {
    float fn;
    fn =  PerlinNoise(uv, density * 1.0f) * 0.5f;
    fn += PerlinNoise(uv, density * 2.0f) * 0.25f;
    fn += PerlinNoise(uv, density * 4.0f) * 0.125f;
    return fn;
}

float32_t3 ApplyNoiseHeightFog(float32_t3 color, float32_t3 position) {
    float32_t heightDiff = position.y - g_Constant.cameraPosition.y;
    float32_t heightFogFactor = saturate(( heightDiff - g_Constant.hightFogStart) / (g_Constant.hightFogEnd - g_Constant.hightFogStart));
    float32_t2 uv = position.xz * 0.1f;
    //float32_t noise = FractalSumNoise(uv, 4.0f);
    //heightFogFactor *= noise;
    heightFogFactor *= g_Constant.fogFactor;
    return lerp(color, g_Constant.hightFogColor, heightFogFactor);
}

[numthreads(8, 8, 1)]
void main(uint32_t2 DTid : SV_DispatchThreadID) {
    float32_t width, height;
    g_Texture.GetDimensions(width, height);
    float32_t2 uv = DTid.xy / float2(width, height);
    
    float32_t3 color = g_Texture[DTid].xyz;
    float32_t3 position = GetWorldPosition(uv, g_Depth[DTid]);
    float32_t depth = length(position - g_Constant.cameraPosition);
    float32_t fogFactor = saturate((depth - g_Constant.depthFogStart) / (g_Constant.depthFogEnd - g_Constant.depthFogStart));
    fogFactor *= g_Constant.fogFactor;
    float32_t3 finalColor = lerp(color, g_Constant.color, fogFactor);
    
    finalColor = ApplyNoiseHeightFog(finalColor, position);

    //float32_t3 finalColor = float32_t3(FractalSumNoise(uv, 10.0f), 0.0f, 0.0f);
    
    g_Texture[DTid].xyz = finalColor;
}