struct VSIn
{
	float4 Pos : Position;
	float4 Color : Color;
    float2 UV : UV;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float4 WorldPos : POSITION;
    float4 Color : Color;
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