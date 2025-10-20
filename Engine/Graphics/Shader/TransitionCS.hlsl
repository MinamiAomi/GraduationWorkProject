struct Parameter {
    float time;
    float3 fadeColor;
};

RWTexture2D<float4> g_Texture : register(u0);
ConstantBuffer<Parameter> g_Parameter : register(b0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    
    //float2 textureSize;
    //g_Texture.GetDimensions(textureSize.x, textureSize.y);
    //float2 uv = DTid.xy / textureSize;
    
    float4 color = g_Texture[DTid.xy];
    
    color.rgb = lerp(color.rgb, g_Parameter.fadeColor, g_Parameter.time);
    
    g_Texture[DTid.xy] = color;
}