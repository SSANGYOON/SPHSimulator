#include "global.hlsli"

struct ParticleVSOut
{
    float4 Pos : SV_Position;
    float3 WorldPos : POSITION;
    float4 Color : Color;
    float2 UV : TEXCOORD;
};

VSOut VS_MAIN(ParticleVSIn In)
{
    VSOut Out = (VSOut)0.f;
    Out.UV = In.UV;


    float4 worldPos = mul(float4(In.Pos, 1.f), In.matW);

    Out.WorldPos = worldPos.xyz;
    Out.Pos = mul(mul(worldPos, view), projection);
    Out.Color = In.Color;

    return Out;
}

float4 PS_MAIN(VSOut In) : SV_Target
{
    float3 irradiance = DiffuseColor * 0.3f + DiffuseColor * LightColor * max(0,dot(LightDirection,normalize(In.Normal)));

    float4 finalColor = float4(irradiance,1);
    return finalColor;
}