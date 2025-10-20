struct VSInput {
    uint vertexID : SV_VertexID;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

// MiniEngine　ScreenQuadCommonVS.hlsl
VSOutput main(VSInput input) {
    VSOutput output;
    output.texcoord = float2(uint2(input.vertexID, input.vertexID << 1) & 2);
    output.position = float4(lerp(float2(-1.0f, 1.0f), float2(1.0f, -1.0f), output.texcoord), 0.0f, 1.0f);
    return output;
}