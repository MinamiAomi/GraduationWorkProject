RWTexture2D<float32_t4> g_Texture : register(u0);
struct Constant {
    float32_t3 color;
};

ConstantBuffer<Constant> g_Constant : register(b0);

[numthreads(8, 8, 1)]
void main(uint32_t2 DTid : SV_DispatchThreadID) {
    float32_t3 color = g_Texture[DTid].xyz;
    float32_t3 k = float32_t3(0.2125f, 0.7154f, 0.0721f);
    float32_t grayscale = dot(color, k);
    g_Texture[DTid].xyz = grayscale * g_Constant.color;
}