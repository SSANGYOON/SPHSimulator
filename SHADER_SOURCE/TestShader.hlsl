#include "global.hlsli"

VSOut VS_MAIN(VSIn In)
{
    VSOut Out = (VSOut)0.f;
    Out.UV = In.UV;

    In.Pos.z = In.Pos.z + 0.1f;

    Out.WorldPos = In.Pos;
    Out.Pos = float4(In.Pos, 1.f);
    Out.Color = In.Color;

    return Out;
}

float4 PS_MAIN(VSOut In) : SV_Target
{
    float4 color = In.Color;

    return color;
}