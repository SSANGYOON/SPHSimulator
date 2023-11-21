#include "Global_SPH.hlsli"

RWStructuredBuffer<float> DivergenceError : register(u6);

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum || DivergenceError[0] <= -100)
		return;

	Particle pi = Particles[piIndex];

	int3 cell = int3((pi.position + boundarySize * 0.5f) / (2 * radius));

	float3 a = (float3)0.f;
	//From Fluid
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			for (int z = -1; z <= 1; z++) {
				uint cellHash = GetHash(cell + int3(x, y, z));
				uint pjIndex = neighborTable[cellHash];
				if (pjIndex == NO_PARTICLE) {
					continue;
				}
				while (pjIndex < particlesNum) {
					if (pjIndex == piIndex) {
						pjIndex++;
						continue;
					}
					Particle pj = Particles[pjIndex];
					if (pj.hash != cellHash) {
						break;
					}

					float3 diff = pi.position - pj.position;
					float dist = length(diff);

					if (dist < 2 * radius && dist > 1e-5f) {
						a += mass * (pi.divergenceStiffness + pj.divergenceStiffness) * cubic_spline_kernel_gradient(diff);
					}
					pjIndex++;
				}
			}
		}
	}

	//boundary handling
	float3 boundaryLocal = (pi.position - obstaclePos - obstacleOffset) / radius;

	if ((boundaryLocal.x > 0 && boundaryLocal.x < obstacleSize.x) &&
		(boundaryLocal.y > 0 && boundaryLocal.y < obstacleSize.y) &&
		(boundaryLocal.z > 0 && boundaryLocal.z < obstacleSize.z))
	{
		float boundarySDF = triLinearSDF(boundaryLocal);
		float3 boundaryVolume = triLinearVolume(boundaryLocal);
		float3 normal = normalize(sdfGradient(boundaryLocal));

		float3 diff = clamp(boundarySDF, 1e-3, 3 * radius) * normal;

		a += boundaryVolume * restDensity * pi.divergenceStiffness * cubic_spline_kernel_gradient(diff);
	}

	Particles[piIndex].velocity -= a;
}