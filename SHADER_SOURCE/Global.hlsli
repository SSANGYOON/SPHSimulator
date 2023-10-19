struct VSIn
{
	float3 Pos : POSITION;
    float2 UV : UV;
    float3 Normal : NORMAL;
	float4 Color : COLOR; 
};

struct ParticleVSIn
{
    float3 Pos : POSITION;
    float2 UV : UV;
    float3 Normal : NORMAL;
    float4 Color : COLOR;

    row_major matrix matW : W;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float3 WorldPos : POSITION;
    float4 Color : Color;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

cbuffer Transform : register(b0)
{
    row_major matrix world;
    row_major matrix view;
    row_major matrix projection;
}

cbuffer TextureBind : register(b1)
{
    int     tex0_On;
    int     tex1_On;
    int     tex2_On;
    int     tex3_On;
}