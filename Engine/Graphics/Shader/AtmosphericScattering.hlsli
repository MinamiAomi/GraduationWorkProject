#define M_MAX 1e9
static const float32_t M_PI = 3.14159265359f;
static const float32_t M_4PI = 4.0f * M_PI;

struct SkyParameter {
    float32_t3 sunPosition;
    float32_t sunIntensity;
    
    float32_t Kr;
    float32_t Km;
    float32_t innerRadius;
    float32_t outerRadius;

    float32_t3 invWaveLength;
    float32_t scale;
    
    float32_t scaleDepth;
    float32_t scaleOverScaleDepth;
    float32_t g;
    float32_t exposure;
};

float32_t3 IntersectionSphere(float32_t3 rayOrigin, float32_t3 rayDirection, float32_t sphereRadius) {
    float32_t a = dot(rayDirection, rayDirection);
    float32_t b = 2.0f * dot(rayOrigin, rayDirection);
    float32_t c = dot(rayOrigin, rayOrigin) - sphereRadius * sphereRadius;
    float32_t d = b * b - 4.0f * a * c;

    if (d < 0.0001f) { return float32_t3(0.0f, 0.0f, 0.0f); }
    return (rayOrigin + rayDirection * (0.5f * (-b + sqrt(d)) / a));
}

float32_t IntegralApproximation(float32_t cosine, float32_t scaleDepth) {
    float32_t x = 1.0f - cosine;
	return scaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
}

float32_t3 AtmosphericScattering(float32_t3 rayOrigin, float32_t3 rayDirection, SkyParameter skyParameter) {
    rayOrigin.y += skyParameter.innerRadius;
    float32_t3 skyPosition = IntersectionSphere(rayOrigin, rayDirection, skyParameter.outerRadius);
    float32_t skyDistance = length(skyPosition - rayOrigin);

    float32_t height = length(rayOrigin) + 0.0001f;
    float32_t depth = exp(skyParameter.scaleOverScaleDepth * (skyParameter.innerRadius - height));
    float32_t angle = dot(rayDirection, rayOrigin) / height;
    float32_t offset = depth * IntegralApproximation(angle, skyParameter.scaleDepth);

    const int32_t numSamples = 5;
    float32_t sampleStep = skyDistance / (float32_t)numSamples;
    float32_t scaledSampleStep = sampleStep * skyParameter.scale;
    float32_t3 samplePoint = rayOrigin + rayDirection * 0.5f;
    const float32_t3 k = skyParameter.invWaveLength * (skyParameter.Kr + skyParameter.Km) * M_4PI;

    float32_t3 frontColor = float32_t3(0.0f, 0.0f, 0.0f);
    for (int32_t i = 0; i < numSamples; ++i) {
        height = length(samplePoint);
        depth = exp(skyParameter.scaleOverScaleDepth * (skyParameter.innerRadius - height));
        float32_t sunAngle = dot(normalize(skyParameter.sunPosition), samplePoint) / height;
        float32_t rayAngle = dot(normalize(rayDirection), samplePoint) / height;
        float32_t scatter = (offset + depth * (IntegralApproximation(sunAngle, skyParameter.scaleDepth) - IntegralApproximation(rayAngle, skyParameter.scaleDepth)));
        float32_t3 attenuate = exp(-scatter * k);
        frontColor += attenuate * (depth * scaledSampleStep);
        samplePoint += sampleStep * rayDirection;
    }

    float32_t3 primaryColor = frontColor * (skyParameter.invWaveLength * skyParameter.Kr * skyParameter.sunIntensity);
    float32_t3 secondaryColor = frontColor * (skyParameter.Km * skyParameter.sunIntensity);

    float32_t cosine = dot(skyParameter.sunPosition, normalize(rayOrigin - skyPosition));
    float32_t cosine2 = cosine * cosine;
    float32_t g = skyParameter.g;
    float32_t g2 = g * g;

    float32_t rayleightPhase = 0.75f * (1.0f + cosine2);
    float32_t miePhase = 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + cosine2) / pow(abs(1.0f + g2 - 2.0f * g * cosine), 1.5f);

    float32_t3 color = saturate(1.0f - exp(-skyParameter.exposure * (primaryColor * rayleightPhase + secondaryColor * miePhase)));
    return color;
}