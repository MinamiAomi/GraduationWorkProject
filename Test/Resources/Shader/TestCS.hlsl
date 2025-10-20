struct Constant {
    float time;
};

RWTexture2D<float4> g_Texture : register(u0);
ConstantBuffer<Constant> g_Constant : register(b0);

float2 mod(float2 x, float2 y) {
    return x - y * floor(x / y);
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
    fn = PerlinNoise(uv, density * 1.0f);
    //fn += PerlinNoise(uv, density * 2.0f) * 1.0f /  4.0f;
    //fn += PerlinNoise(uv, density * 4.0f) * 1.0f /  8.0f;
    //fn += PerlinNoise(uv, density * 8.0f) * 1.0f / 16.0f;
    return fn;
}


float4 NoiseTest(in float2 uv) {
    float random = FractalSumNoise(uv, 10.0f);
    return lerp(float4(1.0f, 1.0f, 1.0f, 1.0f), float4(0.2f, 0.3f, 0.6f, 1.0f), 1.0f - pow(1.0f - random, 3.0f));
}

float4 Hexagon(in float2 uv) {
    
    uv = uv * 2.0f - 1.0f;
    
    uv *= 4.0;

    float3 color = 0.0f;
    
    float2 r = normalize(float2(1.0f, 1.73f));
    float2 h = r * 0.5f;
    float2 a = mod(uv, r) - h;
    float2 b = mod(uv - h, r) - h;

    float2 gv = length(a) < length(b) ? a : b;
    float2 id = uv - gv;
    
    return float4(color, 1.0f);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    
    float2 textureSize;
    g_Texture.GetDimensions(textureSize.x, textureSize.y);
    float2 uv = DTid.xy / textureSize;
    //g_Texture[DTid.xy] = Hexagon(uv);
        
    float r = PerlinNoise(uv, 64.0f);
    if (r <= 0.2f) {
    g_Texture[DTid.xy] = float4(float3(1.0f, 0, 0), 1.0f);
    }
    else if (r >= 0.8f) {
        g_Texture[DTid.xy] = float4(float3(0,1,0), 1.0f);
    }
    else {
        g_Texture[DTid.xy] = float4(float3(0,0,0), 1.0f);
    }
}