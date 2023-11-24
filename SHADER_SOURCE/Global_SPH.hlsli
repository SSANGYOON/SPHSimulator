const static uint GroupThreadNum = 256;
const static uint NO_PARTICLE = 0xFFFFFFFF;
const static float PI = 3.14159265358979323846f;
const static float ITERATIONEND = -1000;

struct Particle
{
	float3 position;
	float density;
	float3 velocity;
	float alpha;
	uint hash;
	float densityStiffness;
	float divergenceStiffness;
	float3 acceleration;
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

RWStructuredBuffer<float> sdfGrid : register(u4);
RWStructuredBuffer<float> volumeGrid : register(u5);

cbuffer ParticleSettings : register(b2)
{
	uint particlesNum;
	float radius;
	float restDensity;
	float mass;
	float3 boundaryCentor;
	float viscosity;
	float3 boundarySize;
	float gravity;
	float deltaTime;
	uint tableSize;
	float2 settingsPadding;

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

cbuffer ObstacleCB : register(b6)
{
	float3 obstaclePos;
	float obsPadding1;
	float3 obstacleOffset;
	float obsPadding2;
	uint3 obstacleSize;
	float obsPadding3;
	row_major matrix obstacleRotation;
};

uint toLinear(uint x, uint y, uint z)
{
	return z * obstacleSize.x * obstacleSize.y + y * obstacleSize.x + x;
}

float triLinearSDF(float3 p)
{
	uint x0 = uint(clamp(floor(p.x), 0, obstacleSize.x - 1));
	uint x1 = uint(clamp(ceil(p.x), 0, obstacleSize.x - 1));

	uint y0 = uint(clamp(floor(p.y), 0, obstacleSize.y - 1));
	uint y1 = uint(clamp(ceil(p.y), 0, obstacleSize.y - 1));

	uint z0 = uint(clamp(floor(p.z), 0, obstacleSize.z - 1));
	uint z1 = uint(clamp(ceil(p.z), 0, obstacleSize.z - 1));

	float coeffX = (x1 - p.x);
	float coeffY = (y1 - p.y);
	float coeffZ = (z1 - p.z);

	float temp00 = coeffX * sdfGrid[toLinear(x0, y0, z0)] + (1 - coeffX) * sdfGrid[toLinear(x1, y0, z0)];
	float temp10 = coeffX * sdfGrid[toLinear(x0, y1, z0)] + (1 - coeffX) * sdfGrid[toLinear(x1, y1, z0)];
	float temp0 = coeffY * temp00 + (1 - coeffY) * temp10;

	float temp01 = coeffX * sdfGrid[toLinear(x0, y0, z1)] + (1 - coeffX) * sdfGrid[toLinear(x1, y0, z1)];
	float temp11 = coeffX * sdfGrid[toLinear(x0, y1, z1)] + (1 - coeffX) * sdfGrid[toLinear(x1, y1, z1)];
	float temp1 = coeffY * temp01 + (1 - coeffY) * temp11;

	float r = coeffZ * temp0 + (1 - coeffZ) * temp1;

	return r;
}

float triLinearVolume(float3 p)
{
	uint x0 = uint(clamp(floor(p.x), 0, obstacleSize.x - 1));
	uint x1 = uint(clamp(ceil(p.x), 0, obstacleSize.x - 1));

	uint y0 = uint(clamp(floor(p.y), 0, obstacleSize.y - 1));
	uint y1 = uint(clamp(ceil(p.y), 0, obstacleSize.y - 1));

	uint z0 = uint(clamp(floor(p.z), 0, obstacleSize.z - 1));
	uint z1 = uint(clamp(ceil(p.z), 0, obstacleSize.z - 1));

	float coeffX = (x1 - p.x);
	float coeffY = (y1 - p.y);
	float coeffZ = (z1 - p.z);

	float temp00 = coeffX * volumeGrid[toLinear(x0, y0, z0)] + (1 - coeffX) * volumeGrid[toLinear(x1, y0, z0)];
	float temp10 = coeffX * volumeGrid[toLinear(x0, y1, z0)] + (1 - coeffX) * volumeGrid[toLinear(x1, y1, z0)];
	float temp0 = coeffY * temp00 + (1 - coeffY) * temp10;

	float temp01 = coeffX * volumeGrid[toLinear(x0, y0, z1)] + (1 - coeffX) * volumeGrid[toLinear(x1, y0, z1)];
	float temp11 = coeffX * volumeGrid[toLinear(x0, y1, z1)] + (1 - coeffX) * volumeGrid[toLinear(x1, y1, z1)];
	float temp1 = coeffY * temp01 + (1 - coeffY) * temp11;

	float r = coeffZ * temp0 + (1 - coeffZ) * temp1;

	return r;
}

float3 sdfGradient(float3 p)
{
	const float eps = 1e-5;
	return float3(
		float3(triLinearSDF(p + float3(eps, 0.f, 0.f)), triLinearSDF(p + float3(0.f, eps, 0.f)), triLinearSDF(p + float3(0.f, 0.f, eps)))
	  - float3(triLinearSDF(p - float3(eps, 0.f, 0.f)), triLinearSDF(p - float3(0.f, eps, 0.f)), triLinearSDF(p - float3(0.f, 0.f, eps)))
		) * (0.5f / eps);
}

uint GetHash(int3 cell)
{
	return (uint)((cell.x * 73856093) ^ (cell.y * 19349663) ^ (cell.z * 83492791)) % tableSize;
}

uint GetHashValueOfLocation(float3 position)
{
	int3 cell = int3((position + boundarySize * 0.5f) / (2.01f * radius));

	return GetHash(cell);
}

float cubic_spline_kernel(const float r)
{
	const float q = r / radius;

	if (q > 2.0f) return 0.0f;
	else {
		const float a = 0.25f / (PI * radius * radius * radius);
		return a * ((q > 1.0f) ? (2.0f - q) * (2.0f - q) * (2.0f - q) : ((3.0f * q - 6.0f) * q * q + 4.0f));
	}
}

float3 cubic_spline_kernel_gradient(const float3 r)
{
	const float q = length(r) / radius;

	if (q > 2.0f) return (float3)0.f;
	else {
		const float3 a = 0.25f / (PI * radius * radius * radius * radius) * normalize(r);
		return a * ((q > 1.0f) ? ((2.0f - q) * (2.0f - q) * -3.f) : ((9.0f * q - 12.0f) * q));
	}
}