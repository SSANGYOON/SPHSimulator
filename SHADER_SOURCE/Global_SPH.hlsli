const static uint GroupThreadNum = 256;
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
	uint IndexCountPerInstance;
	uint InstanceCount;
	uint StartIndexLocation;
	int BaseVertexLocation;
	uint StartInstanceLocation;
};

RWStructuredBuffer<Particle> Particles : register(u0);
RWStructuredBuffer<uint> neighborTable : register(u1);
RWStructuredBuffer<IndirectArgs> IndirectBuffer : register(u2);
RWStructuredBuffer<float3> ParticleWorld : register(u3);

RWStructuredBuffer<Particle> boundaryParticles : register(u4);
RWStructuredBuffer<uint> boundaryNeighborTable : register(u5);

cbuffer ParticleSettings : register(b2)
{
	uint particlesNum;
	float radius;
	float gasConstant;
	float restDensity;
	float mass;
	float3 boundaryCentor;
	float viscosity;
	float3 boundarySize;
	float gravity;
	float deltaTime;
	uint tableSize;
	uint boundaryParticlesNum;

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

cbuffer ParticleRender : register(b4)
{
	float blurDepthFalloff;
	int filterRadius;
	float absorbanceCoff;
	float renderPadding;
	float3 SpecularColor;
	float SpecularPower;
	float SpecularIntensity;
	float3 fluidColor;
}


uint GetHash(int3 cell)
{
	return (uint)((cell.x * 73856093) ^ (cell.y * 19349663) ^ (cell.z * 83492791)) % tableSize;
}

uint GetHashValueOfLocation(float3 position)
{
	int3 cell = int3((position + boundarySize * 0.5f) / (2 * radius));

	return GetHash(cell);
}

float cubicspline(float q)
{
	const float coeff = 1.f / (3.141592f * pow(radius, 3));

	if (q < 1.f)
		return coeff * (0.75 * q * q * (q - 2.f) + 1.f);
	else if (q < 2.f)
		return coeff * pow(2.0f - q, 3.0f) * 0.25f;
	else // q >= 2.f
		return 0.0f;
}
float CubicSplineGrad(const float q) 
{
	const float coeff = 1.f / (3.141592f * pow(radius, 3));

	if (q < 1.f)
		return coeff * (2.25f * q - 3.f) * q;
	else if (q < 2.f)
		return coeff * -0.75f * (2.f - q) * (2.f - q);
	else // q >= 2.f
		return 0.0f;
}