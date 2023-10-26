const static uint GroupThreadNum = 1024;
const static uint TABLESIZE = 32768; //262139;
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

struct IndirectArgs {
	uint VertexCountPerInstance;
	uint InstanceCount;
	uint StartVertexLocation;
	uint StartInstanceLocation;
};

RWStructuredBuffer<Particle> Particles : register(u0);
RWStructuredBuffer<uint> neighborTable : register(u1);
RWStructuredBuffer<IndirectArgs> IndirectBuffer : register(u2);
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

	//TODO
	//surfaceTensionConstant
	//surfaceTensionOffset
}

cbuffer ParticleSort : register(b3)
{
	uint j;
	uint k;
	float2 SortPadding;
}

uint GetHash(int3 cell)
{
	return (uint)((cell.x * 73856093) ^ (cell.y * 19349663) ^ (cell.z * 83492791)) % TABLESIZE;
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

float cubicspline(float q)
{
	const float coeff = 3.0f / (2.0f * 3.141592f);
	if (q > 2.f)
	{
		return 0;
	}
	else
	{

		float ans = 0.f;
		if (q < 1.f)
		{
			ans = 2.f / 3.f - q * q + 0.5f * q * q * q;
		}
		else
		{
			float r = 2.f - q;
			ans = r * r * r / 6;
		}

		return coeff * ans;
	}
}

float CubicSplineGrad(float q) {

	float coeff = 3.0f / (2.0f * 3.141592f);

	if (q < 1.0f)
		return coeff * (-2.0f * q + 1.5f * q * q);
	else if (q < 2.0f)
		return coeff * -0.5f * (2.0f - q) * (2.0f - q);
	else // q >= 2.0f
		return 0.0f;
}