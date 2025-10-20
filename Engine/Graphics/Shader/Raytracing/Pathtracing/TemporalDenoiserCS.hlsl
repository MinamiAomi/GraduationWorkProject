#define NUM_THREADS 32

Texture2D<float32_t4> g_IntermadiateBuffer : register(t0);
RWTexture2D<float32_t4> g_DenoisedBuffer : register(u0);

struct Common {
    uint32_t sampleCount;
};
ConstantBuffer<Common> g_Common : register(b0);

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID, uint32_t3 GTid : SV_GroupThreadID) {

    uint32_t2 samplePixel = DTid.xy;

    uint32_t2 textureSize;
    g_IntermadiateBuffer.GetDimensions(textureSize.x, textureSize.y);

     // テクスチャ外の場合何もしない
    if (samplePixel.x >= textureSize.x || samplePixel.y >= textureSize.y) { return; }

    float32_t3 color = g_IntermadiateBuffer[samplePixel].rgb; 
    float32_t3 accumulationColor = g_DenoisedBuffer[samplePixel].rgb * (g_Common.sampleCount - 1);

    accumulationColor += color;
    color = accumulationColor / g_Common.sampleCount;

    g_DenoisedBuffer[samplePixel].rgb = color;
    g_DenoisedBuffer[samplePixel].a = 1.0f;
}