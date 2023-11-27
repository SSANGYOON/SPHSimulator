#include "Global_SPH.hlsli"

RWStructuredBuffer<float> DensityError : register(u6);

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum || DensityError[0] <= ITERATIONEND)
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
						a += mass * (pi.densityStiffness + pj.densityStiffness) * cubic_spline_kernel_gradient(diff);
					}
					pjIndex++;
				}
			}
		}
	}

	//boundary handling
	row_major matrix rotationTranspose = transpose(obstacleRotation);
	float3 boundaryLocal = (mul(float4(pi.position - obstaclePos, 1.f), rotationTranspose).xyz - obstacleOffset) / radius;

	if ((boundaryLocal.x > 0 && boundaryLocal.x < obstacleSize.x - 1) &&
		(boundaryLocal.y > 0 && boundaryLocal.y < obstacleSize.y - 1) &&
		(boundaryLocal.z > 0 && boundaryLocal.z < obstacleSize.z - 1))
	{
		float boundarySDF = triLinearSDF(boundaryLocal);
		float boundaryVolume = triLinearVolume(boundaryLocal);
		float3 grad = sdfGradient(boundaryLocal);

		if (length(grad) > 1e-5)
		{
			float3 normal = normalize(grad);
			normal = mul(float4(normal, 0.f), obstacleRotation).xyz;

			float3 diff = clamp(boundarySDF, 1e-5, 3 * radius) * normal;

			a += boundaryVolume * restDensity * pi.densityStiffness * cubic_spline_kernel_gradient(diff);
		}
	}

	Particles[piIndex].velocity -= a / deltaTime;
}