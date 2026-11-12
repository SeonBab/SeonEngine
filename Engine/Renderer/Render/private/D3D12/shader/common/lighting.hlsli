#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "registers.hlsli"

static const float PI = 3.14159265f;

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float DistributionGGX(float NdotH, float alpha)
{
    const float a2 = alpha * alpha;
    const float d = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
    return a2 / max(PI * d * d, 1e-6f);
}

float GeometrySmith(float NdotV, float NdotL, float rough)
{
    const float k = (rough + 1.0f) * (rough + 1.0f) / 8.0f;
    const float gv = NdotV / (NdotV * (1.0f - k) + k);
    const float gl = NdotL / (NdotL * (1.0f - k) + k);
    return gv * gl;
}

float3 BRDF(float3 N, float3 V, float3 L, float3 albedo, float rough, float metal)
{
    const float3 H = normalize(V + L);
    const float NdotL = saturate(dot(N, L));
    const float NdotV = max(dot(N, V), 1e-4f);
    const float NdotH = saturate(dot(N, H));
    const float VdotH = saturate(dot(V, H));

    const float3 F0 = lerp(0.04f.xxx, albedo, metal);
    const float3 F = FresnelSchlick(VdotH, F0);
    const float D = DistributionGGX(NdotH, rough * rough);
    const float G = GeometrySmith(NdotV, NdotL, rough);

    const float3 spec = (D * G * F) / max(4.0f * NdotV * NdotL, 1e-4f);
    const float3 kd = (1.0f - F) * (1.0f - metal);

    return (kd * albedo / PI + spec) * NdotL;
}

float3 ShadeSurface(float3 P, float3 N, float3 V, float3 albedo, float rough, float metal, float ao)
{
    rough = clamp(rough, 0.04f, 1.0f);
    metal = saturate(metal);

    float3 color = BRDF(N, V, normalize(-g_LDir), albedo, rough, metal) * g_LCol.rgb * PI;

    [unroll]
    for (uint i = 0; i < 8; i++)
    {
        const Light light = g_Lights[ObjLightId(i)];
        if (light.l_Type == 0)
            continue;

        float3 L;
        float atten = 1.0f;

        if (light.l_Type == 1)
        {
            L = normalize(-light.l_Dir);
        }
        else
        {
            const float3 d = light.l_Pos - P;
            const float dist = length(d);
            L = d / max(dist, 1e-4f);

            const float r = max(light.l_Range, 1e-4f);
            const float x = saturate(1.0f - (dist * dist) / (r * r));
            atten = x * x;

            if (light.l_Type == 3)
            {
                const float cosCone = cos(light.l_Angle * 0.5f);
                const float cd = dot(-L, normalize(light.l_Dir));
                atten *= smoothstep(cosCone, cosCone + 0.05f, cd);
            }
        }

        color += BRDF(N, V, L, albedo, rough, metal) * light.l_Col * atten * PI;
    }

    color += g_LAmb.rgb * albedo * ao;

    return color;
}

#endif
