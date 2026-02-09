RWTexture2D<float32_t4> g_Texture : register(u0);

float3 LinearToSRGB(float3 color) {
    float3 sqrt1 = sqrt(color);
    float3 sqrt2 = sqrt(sqrt1);
    float3 sqrt3 = sqrt(sqrt2);
    float3 srgb = 0.662002687 * sqrt1 + 0.684122060 * sqrt2 - 0.323583601 * sqrt3 - 0.0225411470 * color;
    return srgb;
}

[numthreads(8, 8, 1)]
void main(uint32_t2 DTid : SV_DispatchThreadID) {
    float32_t3 color = g_Texture[DTid].xyz;

    float32_t3 finalColor = LinearToSRGB(color);
    
    g_Texture[DTid].xyz = finalColor;
}