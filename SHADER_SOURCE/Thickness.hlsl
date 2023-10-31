#include "global.hlsli"
#include "Global_SPH.hlsli"

const static float thicknessAlpha = 0.1f;

struct PS_OUT
{
    float color : SV_Target;
    float depth : SV_Depth;
};

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

    float3 viewLeft = float3(viewInv._m00, viewInv._m01, viewInv._m02);
    float3 viewUp = float3(viewInv._m10, viewInv._m11, viewInv._m12);

    float4 worldPos = float4((In.Pos.x * viewLeft + In.Pos.y * viewUp) * 0.15f + In.InstancePos, 1.f);

    float4 viewPos = mul(worldPos, view);
    Out.Pos = mul(viewPos, projection);
    Out.ViewPos = mul(float4(In.InstancePos, 1.f), view).xyz;
    Out.Color = In.Color;

    return Out;
}

float4 PS_MAIN(PSIn In) : SV_Target
{
    PS_OUT OUT = (PS_OUT)0;

    // calculate eye-space sphere normal from texture coordinates
    float3 N;
    N.xy = In.UV * 2.0 - 1.0;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0) discard; // kill pixels outside circle
    N.z = sqrt(1.0 - r2);

    float thickness = N.z * radius;
    float4 output = float4(thickness, 0, 0, thicknessAlpha);

    return output;
}