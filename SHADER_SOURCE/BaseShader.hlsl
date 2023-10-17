#include "global.hlsli"

VSOut VS_MAIN(VSIn In)
{
    VSOut Out = (VSOut)0.f;
    Out.UV = In.UV;

    Out.WorldPos = mul(In.Pos, world);
    Out.Pos = mul(mul(mul(In.Pos, world), view), projection);
    Out.Color = In.Color;

    return Out;
}

float4 PS_MAIN(VSOut In) : SV_Target
{
    float4 color = In.Color;

    if (tex0_On == 1)
        color *= tex_0.Sample(pointSampler, In.UV);

    if (color.w == 0.f)
        discard;

    PSOut Out = (PSOut)0.f;
    Out.Color = color;

    return Out;
}