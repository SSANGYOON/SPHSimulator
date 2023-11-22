float cubic_spline(const float r)
{
	if (r > 2.0f) return 0.0f;
	else {
		float ret = (r > 1.0f) ? (2.0f - r) * (2.0f - r) * (2.0f - r) * 0.25f : (3.0f * r - 6.0f) * r * r + 4.0f;
		return ret / 4.f;
	}
}

const static float samplePoint[5] = { 0.148874339f, 0.433395394f, 0.679409568f, 0.865063367f, 0.973906528f };
const static float sampleCoeff[5] = { 0.295524225f, 0.269266719f, 0.219086363f, 0.149451349f, 0.066671344f };
const static float PI = 3.14159265358979323846f;

RWStructuredBuffer<float> sdfGrid : register(u0);
RWStructuredBuffer<float> volumeGrid : register(u1);

cbuffer sdfProperty : register(b5)
{
	uint3 sdfSize;
	float cellSize;
}

uint toLinear(uint x, uint y, uint z)
{
	return z * sdfSize.x * sdfSize.y + y * sdfSize.x + x;
}

void seperateIndex(in uint index, out uint x, out uint y, out uint z)
{
	z = index / (sdfSize.x * sdfSize.y);
	y = (index % (sdfSize.x * sdfSize.y)) / sdfSize.x;
	x = index % sdfSize.x;
}

float triLinearSDF(float3 p)
{
	uint x0 = uint(clamp(floor(p.x), 0, sdfSize.x - 1));
	uint x1 = uint(clamp(ceil(p.x), 0, sdfSize.x - 1));

	uint y0 = uint(clamp(floor(p.y), 0, sdfSize.y - 1));
	uint y1 = uint(clamp(ceil(p.y), 0, sdfSize.y - 1));

	uint z0 = uint(clamp(floor(p.z), 0, sdfSize.z - 1));
	uint z1 = uint(clamp(ceil(p.z), 0, sdfSize.z - 1));

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

float R(float r)
{
	if (r < 0)
	{
		return 1.f;
	}
	else
	{
		return cubic_spline(r);
	}
}

[numthreads(8, 8, 8)]
void CS_MAIN( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= sdfSize.x || DTid.y >= sdfSize.y || DTid.z >= sdfSize.z)
		return;

	float sum = 0.f;
	float h = cellSize;
	
	for (uint i = 0; i < 5; i++)
	{
		for (uint j = 0; j < 5; j++)
		{
			for (uint k = 0; k < 5; k++)
			{
				float r = samplePoint[i];
				float theta = PI * samplePoint[j];
				float phi = samplePoint[k] * PI / 2.f;
	
				float3 diff = r * float3(sin(phi) * cos(theta), cos(phi), sin(phi) * sin(theta));

				float jacobian = r * r * sin(phi);

				float3 p0 = (float3)DTid + diff * float3(1, 1, 1);
				float3 p1 = (float3)DTid + diff * float3(1, 1, -1);
				float3 p2 = (float3)DTid + diff * float3(1, -1, 1);
				float3 p3 = (float3)DTid + diff * float3(1, -1, -1);
				float3 p4 = (float3)DTid + diff * float3(-1, 1, 1);
				float3 p5 = (float3)DTid + diff * float3(-1, 1, -1);
				float3 p6 = (float3)DTid + diff * float3(-1, -1, 1);
				float3 p7 = (float3)DTid + diff * float3(-1, -1, -1);

				sum += sampleCoeff[i] * sampleCoeff[j] * sampleCoeff[k] * 
					 ( R(triLinearSDF(p0) / h) + R(triLinearSDF(p1) / h) + R(triLinearSDF(p2) / h) + R(triLinearSDF(p3) / h) +
					   R(triLinearSDF(p4) / h) + R(triLinearSDF(p5) / h) + R(triLinearSDF(p6) / h) + R(triLinearSDF(p7) / h)) * jacobian;
			}
		}
	}
	
	volumeGrid[toLinear(DTid.x, DTid.y, DTid.z)] = sum * cellSize * cellSize * cellSize * PI * PI * 0.25f;
}