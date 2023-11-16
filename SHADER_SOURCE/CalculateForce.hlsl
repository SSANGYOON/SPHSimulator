#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum)
		return;

	Particle pi = Particles[piIndex];

	int3 cell = int3((pi.position + boundarySize * 0.5f) / (2 * radius));
	float h2 = radius * radius;

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
						//apply pressure force
						float3 gradPressure = pi.density * mass * (pi.pressure / (pi.density * pi.density) + pj.pressure / (pj.density * pj.density)) *
							cubic_spline_kernel_gradient(diff);

						float3 pressureForce = -gradPressure / pi.density;
						pi.force += pressureForce;

						//apply viscosity force
						float3 viscoForce = 2 * viscosity * mass / pj.density * velocityDiff * dot(diff, cubic_spline_kernel_gradient(diff))
													/ (dist * dist + 0.01f * radius * radius);
						pi.force += viscoForce;
					}
					pjIndex++;
				}
			}
		}
	}

	//From Boundary
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			for (int z = -1; z <= 1; z++) {
				uint cellHash = GetHash(cell + int3(x, y, z));
				uint pjIndex = boundaryNeighborTable[cellHash];
				if (pjIndex == NO_PARTICLE) {
					continue;
				}
				while (pjIndex < particlesNum) {
					if (pjIndex == piIndex) {
						pjIndex++;
						continue;
					}
					Particle pj = boundaryParticles[pjIndex];
					if (pj.hash != cellHash) {
						break;
					}

					float3 diff = pi.position - pj.position;
					float3 velocityDiff = pi.velocity - pj.velocity;
					float dist = length(diff);

					if (dist < 2 * radius && dist > 1e-5f) {

						float boundaryParticleMass = restDensity / pj.density;

						//apply pressure force
						float3 gradPressure = pi.density * boundaryParticleMass * (pi.pressure / (pi.density * pi.density)) * cubic_spline_kernel_gradient(diff);

						float3 pressureForce = -gradPressure / pi.density;
						pi.force += pressureForce;

						//apply friction
						float3 friction = (2 * viscosity * radius / (2 * pi.density)) * boundaryParticleMass * min(dot(velocityDiff, diff), 0) / (dist * dist + 0.01f * radius * radius) * cubic_spline_kernel_gradient(diff);
						pi.force += friction;
					}
					pjIndex++;
				}
			}
		}
	}

	Particles[piIndex] = pi;
}