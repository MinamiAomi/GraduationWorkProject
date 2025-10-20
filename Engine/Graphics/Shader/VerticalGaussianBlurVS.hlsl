Texture2D<float4> src_ : register(t0);

struct VSInput {
    uint vertexID : SV_VertexID;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 texcoord[8] : TEXCOORD0;
};

VSOutput main(VSInput input) {
    
    VSOutput output;

    float2 textureSize;
    src_.GetDimensions(textureSize.x, textureSize.y);
    
    float2 base = float2(uint2(input.vertexID, input.vertexID << 1) & 2);
    
#ifdef HORIZONTAL_BLUR
    float2 texelSize = float2(1.0f / textureSize.x, 0.0f);
#else
    float2 texelSize = float2(0.0f, 1.0f / textureSize.y);
#endif
    
    output.position = float4(lerp(float2(-1.0f, 1.0f), float2(1.0f, -1.0f), base), 0.0f, 1.0f);
   
    float2 offset = texelSize;
    float4 basebase = float4(base, base);
    
    for (uint i = 0; i < 8; ++i) {
        output.texcoord[i].xy = offset;
        output.texcoord[i].zw = -offset;
        output.texcoord[i] += basebase;
        offset += texelSize;
    }
    
    return output;
}