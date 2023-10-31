#include "global.hlsli"

TextureCube cubeMap : register(t0);

struct PSIn
{
    float4 Pos : SV_Position;
    float3 WorldPos : Position;
    float3 CameraPos : CamPosition;
};

PSIn VS_MAIN(VSIn In)
{
    PSIn Out = (PSIn)0;
    Out.Pos = float4(In.Pos.xy * 2 * farClip, farClip, farClip);

    float4 viewPos = mul(Out.Pos, projectionInv);
    viewPos.w = 1;
    Out.WorldPos = mul(viewPos, viewInv).xyz;
    Out.CameraPos = mul(float4(0, 0, 0, 1), viewInv).xyz;

    return Out;
}

float4 PS_MAIN(PSIn In) : SV_TARGET
{
    return cubeMap.Sample(linearSampler, normalize(In.WorldPos - In.CameraPos));
}