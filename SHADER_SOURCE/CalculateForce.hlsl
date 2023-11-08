#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum)
		return;

	Particle pi = Particles[piIndex];

	int3 cell = (pi.position / radius + 0.5f * boundarySize);
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

					if (dist < radius && dist > 1e-3f) {

						//apply pressure force
						float3 gradPressure = pi.density * mass * (pi.pressure / (pi.density * pi.density) + pj.pressure / (pj.density * pj.density)) * CubicSplineGrad(dist * 2.f / radius) *
							normalize(diff);

						float3 pressureForce = -mass / pi.density * gradPressure;
						pi.force += pressureForce;

						//apply viscosity force
						float3 laplacianVelocity = 2 * mass / pi.density * (velocityDiff) /
							(dist * dist + 0.01f * radius * radius) *
							CubicSplineGrad(dist * 2.f / radius) * dist;
						float3 viscoForce = mass * viscosity * laplacianVelocity;
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

					if (dist < radius && dist > 1e-3f) {

						//apply pressure force
						float3 gradPressure = pi.density * mass * (pi.pressure / (pi.density * pi.density) + pj.pressure / (pj.density * pj.density)) * CubicSplineGrad(dist * 2.f / radius) *
							normalize(diff);

						float3 pressureForce = -mass / pi.density * gradPressure;
						pi.force += pressureForce;

						//apply viscosity force
						/*float3 laplacianVelocity = 2 * mass / pi.density * (velocityDiff) /
							(dist * dist + 0.01f * radius * radius) *
							CubicSplineGrad(dist * 2.f / radius) * dist;
						float3 viscoForce = mass * viscosity * laplacianVelocity;
						pi.force += viscoForce;*/
					}
					pjIndex++;
				}
			}
		}
	}

	Particles[piIndex] = pi;
}