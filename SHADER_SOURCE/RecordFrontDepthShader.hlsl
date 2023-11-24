#include "global.hlsli"
#include "Global_SPH.hlsli"

struct PS_OUT
{
    float color : SV_Target;
    float depth : SV_DepthLessEqual;
};

struct PSIn
{
    float4 Pos : SV_Position;
    float3 ViewPos : ViewPostion;
    float2 UV : TEXCOORD;
};

PSIn VS_MAIN(ParticleVSIn In)
{
    PSIn Out = (PSIn)0.f;
    Out.UV = In.UV;

    float4 worldPos = float4(In.InstancePos, 1.f);
    float4 viewPos = mul(worldPos, view);
    Out.Pos = mul(viewPos + float4(In.Pos.xy * radius, 0, 0), projection);
    Out.ViewPos = viewPos.xyz;

    return Out;
}

PS_OUT PS_MAIN(PSIn In)
{
    PS_OUT OUT = (PS_OUT)0;

    // calculate eye-space sphere normal from texture coordinates
    float3 N;
    N.xy = In.UV * 2.0 - 1.0;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0) discard; // kill pixels outside circle
    N.z = -sqrt(1.0 - r2);

    // calculate depth
    float4 pixelPos = float4(In.ViewPos + N * radius * 0.5f, 1.0);
    float4 clipSpacePos = mul(pixelPos, projection);

    OUT.depth = clipSpacePos.z / clipSpacePos.w;
    OUT.color = pixelPos.z;

    return OUT;
}