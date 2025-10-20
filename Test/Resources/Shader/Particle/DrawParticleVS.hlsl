#define HLSL_HEADER
#include "Particle.h"

StructuredBuffer<Particle> g_Particles : register(t0);

struct PreView {
    float32_t4x4 viewProjectionMatrix;
    float32_t4x4 billboardMatrix;
};
ConstantBuffer<PreView> g_PreView : register(b0);

struct VSInput {
    float32_t3 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};

struct VSOutput {
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

VSOutput main(VSInput input, uint32_t instanceId : SV_INSTANCEID) {
    VSOutput output;
    Particle particle = g_Particles[instanceId];
    float32_t4x4 worldMatrix = g_PreView.billboardMatrix;
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz = particle.translate;
    output.position = mul(float32_t4(input.position, 1.0f), mul(worldMatrix, g_PreView.viewProjectionMatrix));
    output.texcoord = input.texcoord;
    output.color = particle.color;
    return output;
}