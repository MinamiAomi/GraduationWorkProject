struct PSInput {
    float32_t4 position : SV_POSITION;
    float32_t4 color : COLOR0;
};

struct PSOutput {
    float32_t4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    PSOutput output;
    output.color = input.color;
    return output;
}