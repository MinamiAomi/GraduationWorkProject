struct PSInput {
    float32_t4 svPosition : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

struct PSOutput {
    float32_t4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    PSOutput output;
    float32_t dist = length(input.texcoord * 2.0f - 1.0f);
    output.color = lerp(input.color, float32_t4(input.color.xyz, 0.0f), saturate(dist));
    return output;
}