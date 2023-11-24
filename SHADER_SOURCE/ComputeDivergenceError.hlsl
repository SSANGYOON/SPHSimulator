#include "Global_SPH.hlsli"

RWStructuredBuffer<float> DivergenceError : register(u6);

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum || DivergenceError[0] <= ITERATIONEND)
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
	row_major matrix rotationTranspose = transpose(obstacleRotation);
	float3 boundaryLocal = (mul(float4(pi.position - obstaclePos, 1.f), rotationTranspose).xyz - obstacleOffset) / radius;

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
			normal = mul(float4(normal, 0.f), obstacleRotation).xyz;

			float3 diff = clamp(boundarySDF, 1e-3, 3 * radius) * normal;
			float dist = length(diff);

			error += boundaryVolume * restDensity * dot(pi.velocity, cubic_spline_kernel_gradient(diff));
		}
	}

	// compression is allowed if density is lower than restDensity
	if (pi.density + deltaTime * error < restDensity && pi.density <= restDensity)
		error = 0.0f;
	
	Particles[piIndex].divergenceStiffness = error * pi.alpha;
	DivergenceError[piIndex] = error;
}