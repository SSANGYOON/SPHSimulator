#include "global.hlsli"
Texture2D normalTexture : register(t0);
struct PSIn
{
    float4 Pos : SV_Position;
    float2 UV : TEXCOORD;
};

PSIn VS_MAIN(VSIn In)
{
    PSIn Out = (PSIn)0.f;
    Out.UV = In.UV;

    In.Pos.z = In.Pos.z + 0.1f;

    Out.Pos = float4(In.Pos * 2, 1.f);

    return Out;
}

float4 PS_MAIN(PSIn In) : SV_TARGET
{
	float3 thickness = normalTexture.Sample(pointSampler, In.UV).xyz;
    return float4(thickness.x * 0.5f + 0.5f, thickness.y * 0.5f + 0.5f, thickness.z * 0.5f + 0.5f, 1);
}