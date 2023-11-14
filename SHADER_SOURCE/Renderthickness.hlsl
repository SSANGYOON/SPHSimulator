#include "global.hlsli"
#include "Global_SPH.hlsli"

struct PSIn
{
    float4 Pos : SV_Position;
    float3 ViewPos : ViewPostion;
    float4 Color : Color;
    float2 UV : TEXCOORD;
};

PSIn VS_MAIN(ParticleVSIn In)
{
    PSIn Out = (PSIn)0.f;
    Out.UV = In.UV;

    float4 worldPos = float4(In.InstancePos, 1.f);
    float4 viewPos = mul(worldPos, view) + float4(In.Pos.xy * radius, 0, 0);
    Out.Pos = mul(viewPos, projection);
    Out.ViewPos = mul(float4(In.InstancePos, 1.f), view).xyz;
    Out.Color = In.Color;

    return Out;
}

float PS_MAIN(PSIn In) : SV_Target;
{
    PS_OUT OUT = (PS_OUT)0;

    // calculate eye-space sphere normal from texture coordinates
    float3 N;
    N.xy = In.UV * 2.0 - 1.0;
    float r2 = dot(N.xy, N.xy);

    if (r2 > 1.0) discard; // kill pixels outside circle

    return radius * sqrt(1.0 - r2);
}