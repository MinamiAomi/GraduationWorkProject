Texture2D<float4> g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

static const float ContrastThreshold = 0.0312f;
static const float RelativeThreshold = 0.063f;

static float2 g_TextureSize;
static float2 g_PixelSize;

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

struct LuminanceData {
    float m, n, e, s, w;
    float ne, nw, se, sw;
    float highest, lowest, contrast;
};

struct EdgeData {
    bool isHorizontal;
    float pixelStep;
};

float3 Sample(float2 center, float2 offset) {
    float2 uv = center + offset;
    float3 color = g_Texture.Sample(g_Sampler, uv).rgb;
    return color;
}

float RGBToLuminance(float3 rgb) {
    return dot(rgb, float3(0.299f, 0.587f, 0.114f));
}

LuminanceData SampleLuminanceData(float2 texcoord) {
    float2 offset = g_PixelSize;
    LuminanceData luminanceData;
    luminanceData.m = RGBToLuminance(Sample(texcoord, float2(0.0f, 0.0f)));
    luminanceData.n = RGBToLuminance(Sample(texcoord, float2(0, offset.y)));
    luminanceData.e = RGBToLuminance(Sample(texcoord, float2(offset.x, 0.0f)));
    luminanceData.s = RGBToLuminance(Sample(texcoord, float2(0.0f, -offset.y)));
    luminanceData.w = RGBToLuminance(Sample(texcoord, float2(-offset.x, 0.0f)));
    
    luminanceData.ne = RGBToLuminance(Sample(texcoord, float2(offset.x, offset.y)));
    luminanceData.nw = RGBToLuminance(Sample(texcoord, float2(-offset.x, offset.y)));
    luminanceData.se = RGBToLuminance(Sample(texcoord, float2(offset.x, -offset.y)));
    luminanceData.sw = RGBToLuminance(Sample(texcoord, float2(-offset.x, -offset.y)));
    
    luminanceData.lowest = min(min(min(min(luminanceData.n, luminanceData.e), luminanceData.s), luminanceData.w), luminanceData.m);
    luminanceData.highest = max(max(max(max(luminanceData.n, luminanceData.e), luminanceData.s), luminanceData.w), luminanceData.m);
    luminanceData.contrast = luminanceData.highest - luminanceData.lowest;

    return luminanceData;
}

bool ShouldSkipPixel(LuminanceData luminanceData) {
    float threshold = max(ContrastThreshold, RelativeThreshold * luminanceData.highest);
    return luminanceData.contrast < threshold;
}

float DeterminePixelBlendFactor(LuminanceData luminanceData) {
    float filter = 2.0f * (luminanceData.n + luminanceData.e + luminanceData.s + luminanceData.w);
    filter += luminanceData.ne + luminanceData.nw + luminanceData.se + luminanceData.sw;
    filter *= 1.0f / 12.0f;
    filter = abs(filter - luminanceData.m);
    filter = saturate(filter / luminanceData.contrast);
    float blendFactor = smoothstep(0.0f, 1.0f, filter);
    return blendFactor * blendFactor;
}

EdgeData DetermineEdge(LuminanceData luminanceData) {
    EdgeData edgeData;
    float horizontal =
        abs(luminanceData.n + luminanceData.s - 2.0f * luminanceData.m) * 2.0f +
        abs(luminanceData.ne + luminanceData.se - 2.0f * luminanceData.e) +
        abs(luminanceData.nw + luminanceData.sw - 2.0f * luminanceData.w);
    float vertical =
        abs(luminanceData.e + luminanceData.w - 2.0f * luminanceData.m) * 2.0f +
        abs(luminanceData.ne + luminanceData.nw - 2.0f * luminanceData.n) +
        abs(luminanceData.se + luminanceData.sw - 2.0f * luminanceData.s);
    
    edgeData.isHorizontal = horizontal >= vertical;
    
    float pLuminance = edgeData.isHorizontal ? luminanceData.n : luminanceData.e;
    float nLuminance = edgeData.isHorizontal ? luminanceData.s : luminanceData.w;
    float pGradient = abs(pLuminance - luminanceData.m);
    float nGradient = abs(nLuminance - luminanceData.m);
    
    edgeData.pixelStep = edgeData.isHorizontal ? g_PixelSize.y : g_PixelSize.x;
    if (pGradient < nGradient) {
        edgeData.pixelStep = -edgeData.pixelStep;
    }
    
    return edgeData;
}

float3 FXAA(float2 texcoord) {
    float2 center = texcoord;
    
    g_Texture.GetDimensions(g_TextureSize.x, g_TextureSize.y);
    g_PixelSize = 1.0f / g_TextureSize;
    
    LuminanceData luminanceData = SampleLuminanceData(texcoord);
    
    if (ShouldSkipPixel(luminanceData)) {
        //return 0.0f;
        return g_Texture.Sample(g_Sampler, texcoord).rgb;
    }
    
    float pixelBlend = DeterminePixelBlendFactor(luminanceData);
    EdgeData edgeData = DetermineEdge(luminanceData);
    
    if (edgeData.isHorizontal) {
        texcoord.y += edgeData.pixelStep * pixelBlend;
    }
    else {
        texcoord.x += edgeData.pixelStep * pixelBlend;
    }
    
    //return edgeData.pixelStep < 0 ? float3(1.0f, 0.0f, 0.0f) : 1.0f;
     
    return g_Texture.Sample(g_Sampler, texcoord).rgb;
}


float3 LinearToSRGB(float3 color) {
    float3 sqrt1 = sqrt(color);
    float3 sqrt2 = sqrt(sqrt1);
    float3 sqrt3 = sqrt(sqrt2);
    float3 srgb = 0.662002687 * sqrt1 + 0.684122060 * sqrt2 - 0.323583601 * sqrt3 - 0.0225411470 * color;
    return srgb;
}

PSOutput main(PSInput input) {
    PSOutput output;
    output.color.rgb = FXAA(input.texcoord);
    output.color.a = 1.0f;
    return output;
}