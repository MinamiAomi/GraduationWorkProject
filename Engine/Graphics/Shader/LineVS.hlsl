struct Scene {
    float32_t4x4 viewProjectionMatrix;
};
ConstantBuffer<Scene> g_Scene : register(b0);

struct VSInput {
    float32_t3 position : POSITION0;
    float32_t4 color : COLOR0;
};

struct VSOutput {
    float32_t4 position : SV_POSITION;
    float32_t4 color : COLOR0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(float32_t4(input.position, 1.0f), g_Scene.viewProjectionMatrix);
    output.color = input.color;
    return output;
}