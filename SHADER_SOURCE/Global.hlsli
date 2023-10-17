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

cbuffer ParticleSettings : register(b2)
{
    uint CellSize;
}

uint SPH_CalculateHash(uint3 cellIndex)
{
    const uint p1 = 73856093;   // some large primes 
    const uint p2 = 19349663;
    const uint p3 = 83492791;
    int n = p1 * cellIndex.x ^ p2 * cellIndex.y ^ p3 * cellIndex.z;
    // TODO 나중에 실제 파티클 크기에 맞춰서 바꿀 수 있도록 할 예정
    n %= 524287;
    return n;
}
