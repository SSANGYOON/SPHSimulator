const static uint GroupThreadNum = 256;
const static uint NO_PARTICLE = 0xFFFFFFFF;
const static float PI = 3.14159265358979323846f;

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
	int3 cell = int3((position + boundarySize * 0.5f) / radius);

	return GetHash(cell);
}

float cubic_spline_kernel(const float r)
{
	const float q = 2.0f * r / radius;

	if (q > 2.0f) return 0.0f;
	else {
		const float a = 0.25f / (PI * radius * radius * radius);
		return a * ((q > 1.0f) ? (2.0f - q) * (2.0f - q) * (2.0f - q) : ((3.0f * q - 6.0f) * q * q + 4.0f));
	}
}

float3 cubic_spline_kernel_gradient(const float3 r)
{
	const float q = 2.0f * length(r) / radius;

	if (q > 2.0f) return (float3)0.f;
	else {
		const float a = r / (PI * q * radius * radius * radius * radius * radius);
		return a * ((q > 1.0f) ? ((12.0f - 3.0f * q) * q - 12.0f) : ((9.0f * q - 12.0f) * q));
	}
}

float viscosity_kernel_laplacian(const float r) {
	return (r <= radius) ? (45.0f * (radius - r) / (PI * pow(radius, 6))) : 0.0f;
}