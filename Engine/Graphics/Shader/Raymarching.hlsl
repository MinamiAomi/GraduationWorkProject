#include "Lighting.hlsli"

#define MAX_STEPS 64
#define MAX_DISTANCE 100.0f
#define EPSILON 0.0001f

#define MIN_FOG_DEPTH 50.0f
#define MAX_FOG_DEPTH 100.0f

#define REPETITION(x, y) (x - floor(x / y) * y - y * 0.5f)

#define REPETITION_CYCLE 5.0f

struct Scene {
    float4x4 viewProjectionInverse;
    float3 cameraPosition;
    float time;
};
ConstantBuffer<Scene> g_Scene : register(b0);
RWTexture2D<float4> g_Output : register(u0);

// texcoodとdepthからワールド座標を計算
float3 GetWorldPosition(in float2 texcoord, in float depth, in float4x4 viewProjectionInverseMatrix) {
    // xは0~1から-1~1, yは0~1から1~-1に上下反転
    float2 xy = texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float4 position = float4(xy, depth, 1.0f);
    position = mul(position, viewProjectionInverseMatrix);
    position.xyz /= position.w;
    return position.xyz;
}

float3 HSVToRGB(in float3 hsv) {
    float4 k = float4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    float3 p = abs(frac(hsv.xxx + k.xyz) * 6.0f - k.www);
    return hsv.z * lerp(k.xxx, clamp(p - k.xxx, 0.0f, 1.0f), hsv.y);
}

float mod(float x, float y) {
    return x - y * floor(x / y);
}

float Random(in float2 uv) {
    return frac(sin(dot(uv, float2(12.9898f, 78.233f))) * 43758.5453f);

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

float sdSphere(in float3 p, in float r) {
    return length(p) - r;
}

float sdBox(in float3 p, in float3 b) {
    float3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f));
}

float Foldback(float t) {
    return -abs(2.0f * t - 1.0f) + 1.0f;
}

float GetDistance(in float3 position) {
    //float r = 1.0f;
    //float distance = length(Trans(position)) - r;
    
    // XZ軸に繰り返し
    float2 r = REPETITION(position.xz, REPETITION_CYCLE);
    float3 rayPos = float3(r.x, position.y, r.y);
    //float height = sin(r.x / 5.0f);
    float t = PerlinNoise((float2) (int2) ((position.xz / REPETITION_CYCLE)), 1.0f / REPETITION_CYCLE);
    float height = lerp(0.0f, 5.0f, t);
    float distance = sdBox(rayPos - float3(0.0f, -20.0f + height * 0.5f, 0.0f), float3(REPETITION_CYCLE / 2.0f, height, REPETITION_CYCLE / 2.0f));
    
    return distance;
}

float3 GetNormal(in float3 position) {
    float3 d = float3(0.0001f, -0.0001f, 0.0f);
    
    float3 normal;
    normal.x = GetDistance(position + d.xzz) - GetDistance(position + d.yzz);
    normal.y = GetDistance(position + d.zxz) - GetDistance(position + d.zyz);
    normal.z = GetDistance(position + d.zzx) - GetDistance(position + d.zzy);
    return normalize(normal);
}

float Raymarch(in float3 rayOrigin, in float3 rayDirection) {
    
    float distance = 0.0f;
    float3 rayDirectionInv = 1.0f / rayDirection ;
    
    for (uint i = 0; i < MAX_STEPS; ++i) {
        float3 position = rayOrigin + rayDirection * distance;
        float d = GetDistance(position);
        
        distance += min(min((step(0.0, rayDirection.x) * REPETITION_CYCLE - mod(position.x, REPETITION_CYCLE)) * rayDirectionInv.x, (step(0.0, rayDirection.z) * REPETITION_CYCLE - mod(position.z, REPETITION_CYCLE)) * rayDirectionInv.z) + 0.01f, d);
        
       //distance += d;
        
        if (distance > MAX_DISTANCE || d < EPSILON) {
            break;
        }
    }

    return distance;
}

float3 Fog(in float3 color, in float3 fogColor, in float depth) {
    float t = (depth - MIN_FOG_DEPTH) / (MAX_FOG_DEPTH - MIN_FOG_DEPTH);
    return lerp(color, fogColor, saturate(t));
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {

    float2 dimensions;
    g_Output.GetDimensions(dimensions.x, dimensions.y);
    
    float2 texcoord = ((float2) DTid.xy + 0.5f) / dimensions;
    float3 nearPosition = GetWorldPosition(texcoord, 0.0f, g_Scene.viewProjectionInverse);
    float3 farPosition = GetWorldPosition(texcoord, 1.0f, g_Scene.viewProjectionInverse);
    
    float3 rayOrigin = nearPosition;
    float3 rayDirection = normalize(farPosition - nearPosition);
    float distance = Raymarch(rayOrigin, rayDirection);
    
    float3 lightDirection = normalize(float3(0.2f, -1.0f, 0.3f));
    
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (distance < MAX_DISTANCE) {
        float3 position = rayOrigin + rayDirection * distance;
        float3 normal = GetNormal(position);
        float3 pixelToCamera = normalize(g_Scene.cameraPosition - position);
        color.rgb = Lighting::HalfLambertReflection(normal, lightDirection) + Lighting::BlinnPhongReflection(normal, pixelToCamera, lightDirection, 10.0f);
        float r = PerlinNoise((float2) (int2) ((position.xz / REPETITION_CYCLE)), 1.0f / REPETITION_CYCLE);
        //color.rgb *= lerp(float3(1.0f, 1.0f, 1.0f), float3(1.0f, 0.0f, 0.0f), abs(floor(position.x / REPETITION_CYCLE) + floor(position.z / REPETITION_CYCLE)) % 2);
        color.rgb *= float3(r,r,r);
        
        color.rgb = Fog(color.rgb, float3(0.0f, 0.0f, 0.0f), distance);
        
    }
        
    g_Output[DTid.xy] = color;
}