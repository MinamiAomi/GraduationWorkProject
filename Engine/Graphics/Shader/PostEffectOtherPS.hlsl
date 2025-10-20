struct Constant {
    float time;
};

ConstantBuffer<Constant> g_Constant : register(b0);
Texture2D<float4> g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

float3 LinearToSRGB(float3 color) {
    float3 sqrt1 = sqrt(color);
    float3 sqrt2 = sqrt(sqrt1);
    float3 sqrt3 = sqrt(sqrt2);
    float3 srgb = 0.662002687 * sqrt1 + 0.684122060 * sqrt2 - 0.323583601 * sqrt3 - 0.0225411470 * color;
    return srgb;
}

float2 Random2(float2 fact) {
    const float2 v1 = float2(127.1f, 311.7f);
    const float2 v2 = float2(269.5f, 183.3f);
    
    float2 angle = float2(dot(fact, v1), dot(fact, v2));
    return frac(sin(angle) * 43758.5453123f) * 2.0f - 1.0f;
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
    fn = PerlinNoise(uv, density * 1.0f) * 1.0f / 2.0f;
    fn += PerlinNoise(uv, density * 2.0f) * 1.0f / 4.0f;
    fn += PerlinNoise(uv, density * 4.0f) * 1.0f / 8.0f;
    fn += PerlinNoise(uv, density * 8.0f) * 1.0f / 16.0f;
    return fn;
}
    
PSOutput main(PSInput input) {
    PSOutput output;
    
    float2 uv = input.texcoord;
    //if (uv.x > 0.25f && uv.x < 0.75f &&
    //    uv.y > 0.25f && uv.y < 0.75f) {
    //    uv.y += FractalSumNoise(uv * float2(0.1f, 1.0f) + g_Constant.time * 0.1f, 10.0f) * 0.1f - 0.05f;
    //}
    
    output.color = g_Texture.Sample(g_Sampler, uv);
    
    return output;
}