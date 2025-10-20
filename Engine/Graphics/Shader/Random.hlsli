
float32_t Random3dTo1d(float32_t3 value, float32_t3 mutator = float32_t3(12.9898f, 78.233f, 37.719f)) {
    return frac(sin(dot(sin(value), mutator)) * 143758.5453f);
}

float32_t Random2dTo1d(float32_t2 value, float32_t2 mutator = float32_t2(12.9898f, 78.233f)) {
    return frac(sin(dot(sin(value), mutator)) * 143758.5453f);
}


float32_t Random1dTo1d(float32_t value, float32_t mutator = 0.546f) {
    return frac(sin(value + mutator) * 143758.5453f);
}

float32_t2 Random3dTo2d(float32_t3 value) {
    return float32_t2(
        Random3dTo1d(value, float32_t3(12.989f, 78.233f, 37.719f)),
        Random3dTo1d(value, float32_t3(39.346f, 11.135f, 83.155f))
    );
}

float32_t2 Random2dTo2d(float32_t2 value) {
    return float32_t2(
        Random2dTo1d(value, float32_t2(12.989f, 78.233f)),
        Random2dTo1d(value, float32_t2(39.346f, 11.135f))
    );
}

float32_t2 Random1dTo2d(float32_t1 value) {
    return float32_t2(
        Random1dTo1d(value, 3.9812f),
        Random1dTo1d(value, 7.1536f)
    );
}

float32_t3 Random3dTo3d(float32_t3 value) {
    return float32_t3(
        Random3dTo1d(value, float32_t3(12.989f, 78.233f, 37.719f)),
        Random3dTo1d(value, float32_t3(39.346f, 11.135f, 83.155f)),
        Random3dTo1d(value, float32_t3(73.156f, 52.235f, 09.151f))
    );
}

float32_t3 Random2dTo3d(float32_t2 value) {
    return float32_t3(
        Random2dTo1d(value, float32_t2(12.989f, 78.233f)),
        Random2dTo1d(value, float32_t2(39.346f, 11.135f)),
        Random2dTo1d(value, float32_t2(73.156f, 52.235f))
    );
}

float32_t3 Random1dTo3d(float32_t1 value) {
    return float32_t3(
        Random1dTo1d(value, 3.9812f),
        Random1dTo1d(value, 7.1536f),
        Random1dTo1d(value, 5.7241f)
    );
}

class RandomGenerator {
    float32_t3 seed;
    float32_t3 Generate3d() {
        seed = Random3dTo3d(seed);
        return seed;
    }

    float32_t Generate1d() {
        float32_t rand = Random3dTo1d(seed);
        seed.x = rand;
        return rand;
    }
};

float32_t3 RandomUnitVectorHemisphere(float32_t3 n, RandomGenerator randomGenerator) {
    float32_t z = randomGenerator.Generate1d() * 2.0f - 1.0f;
    float32_t t = randomGenerator.Generate1d() * 2.0f * 3.14159265359f;
    float32_t r = sqrt(1.0f - z * z);

    float32_t x = r * cos(t);
    float32_t y = r * sin(t);

    float32_t3 v = float32_t3(x, y, z);
    if (dot(v, n) < 0.0f) { v = -v; }
    return v;
}