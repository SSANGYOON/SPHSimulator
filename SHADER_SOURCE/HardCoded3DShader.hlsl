#include "global.hlsli"

const static float3 LightDirection =  float3(0.707, -0.707, 0);
const static float3 LightColor = float3(1.0, 1.0, 1.0);
const static float3 DiffuseColor = float3(0.0, 0.5, 0.9);

struct PSIn
{
    float4 Pos : SV_Position;
    float3 WorldPos : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct PSOut
{
    float4 Color : SV_Target0;
    float Depth : SV_Target1;
};

PSIn VS_MAIN(VSIn In)
{
    PSIn Out = (PSIn)0;
    Out.UV = In.UV;

    float4 worldPos = mul(float4(In.Pos, 1.f), world);

    Out.WorldPos = worldPos.xyz;
    Out.Pos = mul(mul(worldPos, view), projection);
    Out.Normal = In.Normal;

    return Out;
}

PSOut PS_MAIN(PSIn In)
{
    // Compute irradiance (sum of ambient & direct lighting)
    float3 irradiance = DiffuseColor * 0.3f + DiffuseColor * LightColor * max(0,dot(-LightDirection,normalize(In.Normal)));

    float4 finalColor = float4(irradiance,1);

    PSOut Out = (PSOut)0;
    Out.Color = finalColor;
    Out.Depth = In.Pos.w;
    return Out;
}