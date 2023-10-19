#include "global.hlsli"

const static float3 LightDirection =  float3(-1, 0, 0);
const static float3 LightColor = float3(1.0, 1.0, 1.0);
const static float3 DiffuseColor = float3(0.0, 0.5, 0.9);

VSOut VS_MAIN(ParticleVSIn In)
{
    VSOut Out = (VSOut)0.f;
    Out.UV = In.UV;

    float4 worldPos = mul(float4(In.Pos, 1.f), In.matW);

    Out.WorldPos = worldPos.xyz;
    Out.Pos = mul(mul(worldPos, view), projection);
    Out.Normal = mul(float4(In.Normal, 0.f), world).xyz;
    Out.Color = In.Color;

    return Out;
}

float4 PS_MAIN(VSOut In) : SV_Target
{
    // Compute irradiance (sum of ambient & direct lighting)
    float3 irradiance = DiffuseColor * 0.3f + DiffuseColor * LightColor * max(0,dot(LightDirection,normalize(In.Normal)));

    float4 finalColor = float4(irradiance,1);
    return finalColor;
}