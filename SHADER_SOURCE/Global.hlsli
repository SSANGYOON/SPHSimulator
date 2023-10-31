struct VSIn
{
	float3 Pos : POSITION;
    float2 UV : UV;
	float4 Color : COLOR; 
};

struct ParticleVSIn
{
    float3 Pos : POSITION;
    float2 UV : UV;
    float4 Color : COLOR;

    float3 InstancePos : IPOS;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float3 WorldPos : POSITION;
    float4 Color : Color;
    float2 UV : TEXCOORD;
};

cbuffer Transform : register(b0)
{
    row_major matrix world;
    row_major matrix view;
    row_major matrix viewInv;
    row_major matrix projection;
    row_major matrix projectionInv;
}

cbuffer PiplineSetting : register(b1)
{
    int     tex0_On;
    int     tex1_On;
    int     tex2_On;
    int     tex3_On;

    float2 viewPort;
    float nearClip;
    float farClip;
}

SamplerState pointSampler : register(s0);
SamplerState linearSampler : register(s1);
SamplerState anisotropicSampler : register(s2);

Texture2D viewDepthTexture : register(t0);
Texture2D tex_1 : register(t1);
Texture2D tex_2 : register(t2);
Texture2D tex_3 : register(t3);