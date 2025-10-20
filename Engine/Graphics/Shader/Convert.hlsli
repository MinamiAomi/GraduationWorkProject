
// https://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
float32_t3 LinearToSRGB(in float32_t3 rgb) {
    float32_t3 sqrt1 = sqrt(rgb);
    float32_t3 sqrt2 = sqrt(sqrt1);
    float32_t3 sqrt3 = sqrt(sqrt2);
    float32_t3 sRGB = 0.662002687f * sqrt1 + 0.684122060f * sqrt2 - 0.323583601f * sqrt3 - 0.022541147f * rgb;
    return sRGB;
}

float32_t3 SRGBToLinear(in float32_t3 sRGB) {
    float32_t3 rgb = sRGB * (sRGB * (sRGB * 0.305306011f + 0.682171111f) + 0.012522878f);
    return rgb;
}

// https://web.archive.org/web/20200207113336/http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
float32_t3 RGBToHSV(in float32_t3 rgb) {
    float32_t4 k = float32_t4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
    float32_t4 p = lerp(float32_t4(rgb.bg, k.wz), float32_t4(rgb.gb, k.xy), step(rgb.b, rgb.g));
    float32_t4 q = lerp(float32_t4(p.xyw, rgb.r), float32_t4(rgb.r, p.yzx), step(p.x, rgb.r));
    float32_t d = q.x - min(q.w, q.y);
    float32_t e = 1.0e-10f;
    return float32_t3(abs(q.z + (q.w - q.y) / (6.0f * d + e)), d / (q.x + e), q.x);
}

// https://web.archive.org/web/20200207113336/http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
float32_t3 HSVToRGB(in float32_t3 hsv) {
    float32_t4 k = float32_t4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    float32_t3 p = abs(frac(hsv.xxx + k.xyz) * 6.0f - k.www);
    return hsv.z * lerp(k.xxx, clamp(p - k.xxx, 0.0f, 1.0f), hsv.y);
}
