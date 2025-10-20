#include "Convert.hlsli"

struct Constant {
    float32_t3 grayscaleColor;
    uint32_t useGrayscale;
    float32_t vignetteIntensity;
    float32_t vignettePower;
    uint32_t useVignette;
    uint32_t pad1;

    float32_t3 hsvBias;
    uint32_t pad2;
    float32_t3 hsvFactor;
};
ConstantBuffer<Constant> g_Constant : register(b0);
Texture2D<float32_t4> g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PSInput {
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float32_t4 color : SV_TARGET0;
};

float32_t3 Grayscale(float32_t3 color) {
    float32_t3 k = float32_t3(0.2125f, 0.7154f, 0.0721f);
    return dot(color, k) * g_Constant.grayscaleColor;
}

float32_t Vignette(float32_t2 texcoord) {
    float32_t2 correct = texcoord * (1.0f - texcoord.yx);
    float32_t vignette = correct.x * correct.y * g_Constant.vignetteIntensity;
    return saturate(pow(vignette, g_Constant.vignettePower));
}

float32_t WrapValue(float32_t value, float32_t minRange, float32_t maxRange) {
    float32_t range = maxRange - minRange;
    float32_t modValue = fmod(value - minRange, range);
    if (modValue < 0.0f) { modValue += range; }
    return minRange + modValue;
}

PSOutput main(PSInput input) {
    PSOutput output;

    float32_t4 outputColor = g_Texture.Sample(g_Sampler, input.texcoord);

    float32_t3 grayscale = Grayscale(outputColor.rgb);
    outputColor.rgb = lerp(outputColor.rgb, grayscale, g_Constant.useGrayscale);

    float32_t vignette = Vignette(input.texcoord);
    outputColor.rgb *= lerp(1.0f, vignette, g_Constant.useVignette);

    float32_t3 hsv = RGBToHSV(outputColor.rgb);
    hsv = hsv * g_Constant.hsvFactor + g_Constant.hsvBias;
    hsv.x = WrapValue(hsv.x, 0.0f, 1.0f);
    hsv.yz = saturate(hsv.yz);
    outputColor.rgb = HSVToRGB(hsv);

    // そのままスワップチェーンにコピーしているのでここでSRGBに変換
    //outputColor.rgb = LinearToSRGB(outputColor.rgb);


    output.color = outputColor;

    return output;
}