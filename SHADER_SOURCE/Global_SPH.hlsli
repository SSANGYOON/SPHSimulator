const static uint GroupThreadNum = 1024;
const static uint TABLESIZE = 4096; //262139;
const static uint NO_PARTICLE = 0xFFFFFFFF;
struct Particle
{
	float3 position;
	float density;
	float3 velocity;
	float pressure;
	uint hash;
	float3 force;
};

RWStructuredBuffer<Particle> Particles : register(u0);
RWStructuredBuffer<uint> neighborTable : register(u1);
RWStructuredBuffer<uint> aliveParticles : register(u2);
RWStructuredBuffer<float4x4> ParticleWorld : register(u3);

groupshared uint LocalPrefixSum[GroupThreadNum];

cbuffer ParticleSettings : register(b2)
{
	uint particlesNum;
	float radius;
	float massPoly6Product;
	float selfDens;
	float gasConstant;
	float restDensity;
	float mass;
	float spikyGrad;
	float spikyLap;
	float viscosity;
	float gravity;
	float deltaTime;
}

cbuffer ParticleSort : register(b3)
{
	uint j;
	uint k;
}

uint GetHash(int3 cell)
{
	return (uint)((cell.x * 73856093) ^ (cell.y * 19349663) ^ (cell.x * 83492791)) % TABLESIZE;
}

uint GetHashValueOfLocation(float3 position)
{
	int3 cell = position / radius;

	return GetHash(cell);
}

uint ExclusiveScan(uint gIdx, uint gId)
{

	uint jump = 1;
	uint n = GroupThreadNum >> 1;
	for (n = GroupThreadNum >> 1; n > 0; jump <<= 1, n >>= 1)
	{
		if (gIdx < n)
		{
			uint fromInd = jump * (2 * gIdx + 1) - 1;
			uint toInd = jump * (2 * gIdx + 2) - 1;
			LocalPrefixSum[toInd] += LocalPrefixSum[fromInd];
		}
		GroupMemoryBarrierWithGroupSync();
	}

	uint GroupSum = 0;
	if (gIdx == 0)
	{
		GroupSum = LocalPrefixSum[GroupThreadNum - 1];
		LocalPrefixSum[GroupThreadNum - 1] = 0;
	}

	GroupMemoryBarrierWithGroupSync();
	jump >>= 1;
	for (n = 1; n < GroupThreadNum; jump >>= 1, n <<= 1)
	{
		if (gIdx < n)
		{
			uint fromInd = jump * (2 * gIdx + 1) - 1;
			uint toInd = jump * (2 * gIdx + 2) - 1;

			uint temp = LocalPrefixSum[toInd];
			LocalPrefixSum[toInd] += LocalPrefixSum[fromInd];
			LocalPrefixSum[fromInd] = temp;
		}
		GroupMemoryBarrierWithGroupSync();
	}

	return GroupSum;
}