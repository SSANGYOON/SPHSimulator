#include "global.hlsli"

VSOut VS_MAIN(VSIn In)
{
    VSOut Out = (VSOut)0.f;
    Out.UV = In.UV;

    float4 worldPos = mul(float4(In.Pos, 1.f), world);

    Out.WorldPos = worldPos.xyz;
    Out.Pos = mul(mul(worldPos, view), projection);
    return Out;
}

float4 PS_MAIN(VSOut In) : SV_TARGET
{
    return float4(0,1,0,1);
}