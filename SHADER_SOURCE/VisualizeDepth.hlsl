#include "global.hlsli"

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
	float depth = viewDepthTexture.Sample(pointSampler, In.UV).x;
    return float4(depth, depth, depth, 1);
}