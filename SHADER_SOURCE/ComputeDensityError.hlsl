#include "Global_SPH.hlsli"

RWStructuredBuffer<float> DensityError : register(u6);

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum || DensityError[0] <= -100)
		return;

	Particle pi = Particles[piIndex];

	int3 cell = int3((pi.position + boundarySize * 0.5f) / (2 * radius));
	float error = 0.f;

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
					float3 velocityDiff = pi.velocity - pj.velocity;
					float dist = length(diff);

					if (dist < 2 * radius && dist > 1e-5f) {
						error += mass * dot(velocityDiff, cubic_spline_kernel_gradient(diff));
					}
					pjIndex++;
				}
			}
		}
	}

	//boundary handling
	float3 boundaryLocal = (pi.position - obstaclePos - obstacleOffset) / radius;

	if ((boundaryLocal.x > 0 && boundaryLocal.x < obstacleSize.x - 1) &&
		(boundaryLocal.y > 0 && boundaryLocal.y < obstacleSize.y - 1) &&
		(boundaryLocal.z > 0 && boundaryLocal.z < obstacleSize.z - 1))
	{
		float boundarySDF = triLinearSDF(boundaryLocal);
		float boundaryVolume = triLinearVolume(boundaryLocal);
		float3 grad = sdfGradient(boundaryLocal);

		if (length(grad) > 1e-3)
		{
			float3 normal = normalize(grad);

			float3 diff = clamp(boundarySDF, 1e-3, 3 * radius) * normal;
			float dist = length(diff);

			error += boundaryVolume * restDensity * dot(pi.velocity, cubic_spline_kernel_gradient(diff));
		}
	}

	error = max(pi.density - restDensity + error * deltaTime, 0.f);
	DensityError[piIndex] = error;
	Particles[piIndex].densityStiffness = error * pi.alpha;
}