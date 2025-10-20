struct Surface {
    float3 position;
    float3 normal;
    float3 directionToCamera;
    float3 distanceToCamera;
};

namespace Lighting {

    struct DirectionalLight {
        float3 color;
        float3 direction;
        float intensity;
    };

    struct PointLight {
        float3 color;
        float3 position;
        float intensity;
        float radius;
        
        // http://www.cemyuksel.com/research/pointlightattenuation/
        float AttenuationFactor(in float3 worldPosition) {
            float distance = length(worldPosition - position);
            return saturate(2.0f / radius * radius * (1.0f - distance / sqrt(distance * distance + radius * radius)));
        }
    };

    struct SpotLight {
        float3 position;
        float3 direction;
        float3 attenuation;
        float attenuationStartAngleCos;
        float attenuationEndAngleCos;
        float3 color;
        float3 intensity;
    };

    float LambertReflection(in float3 normal, in float3 lightDirection) {
        return saturate(dot(normal, -lightDirection));
    }

    float HalfLambertReflection(in float3 normal, in float3 lightDirection) {
        return pow(dot(normal, -lightDirection) * 0.5f + 0.5f, 2.0f);
    }

    float PhongReflection(in float3 normal, in float3 lightDirection, in float3 toCamera, in float shininess) {
        float3 reflectVec = reflect(-lightDirection, normal);
        return pow(saturate(dot(reflectVec, toCamera)), shininess);
    }

    float BlinnPhongReflection(in float3 normal, in float3 lightDirection, in float3 toCamera, in float shininess) {
        float3 halfVec = normalize(-lightDirection + toCamera);
        return pow(saturate(dot(normal, halfVec)), shininess);
    }
    
}