#include "global.hlsli"
#include "Global_SPH.hlsli"

Texture2D<float> obstacleDepthMap: register(t0);

struct PSIn
{
    float4 Pos : SV_Position;
    float3 ViewPos : ViewPostion;
    float2 UV : TEXCOORD;
    float2 SampleUV : SAMPLEUV;
};

PSIn VS_MAIN(ParticleVSIn In)
{
    PSIn Out = (PSIn)0.f;
    Out.UV = In.UV;

    float4 worldPos = float4(In.InstancePos, 1.f);
    float4 viewPos = mul(worldPos, view);
    Out.Pos = mul(viewPos + float4(In.Pos.xy * radius, 0, 0), projection);
    Out.ViewPos = viewPos.xyz;
    Out.SampleUV = float2((Out.Pos.x / Out.Pos.w + 1.0f) * 0.5f, (1.0f - Out.Pos.y / Out.Pos.w) * 0.5f);
    return Out;
}

float4 PS_MAIN(PSIn In) : SV_Target
{
    // calculate eye-space sphere normal from texture coordinates
    float2 N = In.UV * 2.0 - 1.0;
    float r2 = dot(N, N);

    if (r2 > 1.0) discard; // kill pixels outside circle

    float minz = In.Pos.w - radius * 0.5f * sqrt(1.0 - r2);
    float maxz = In.Pos.w + radius * 0.5f * sqrt(1.0 - r2);
    
    float obstacleDepth = obstacleDepthMap.Sample(linearSampler, In.SampleUV);

    return float4(min(maxz, obstacleDepth) - min(minz, obstacleDepth), 0, 0, 1.f);
}