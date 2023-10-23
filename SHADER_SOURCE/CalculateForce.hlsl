#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum)
		return;

	Particle pi = Particles[piIndex];

	float3 force = (float3)0;

	int3 cell = pi.position / radius;
	float h2 = radius * radius;

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
					float dist = length(pj.position - pi.position);
					if (dist < radius && dist > 1e-3f) {
						//unit direction and length
						float3 dir = normalize(pj.position - pi.position);

						//apply pressure force
						float3 pressureForce = -dir * mass *
							(pi.pressure / (pi.density * pi.density)
						   + pj.pressure / (pj.density * pj.density));

						pressureForce *= CubicSplineGrad(2 * dist / radius);
						force += pressureForce;

						//apply viscosity force
						float3 velocityDif = pj.velocity - pi.velocity;
						float3 viscoForce = viscosity * mass * (velocityDif / pj.density) * spikyLap * (radius - dist);
						force += viscoForce;
					}
					pjIndex++;
				}
			}
		}
	}
	pi.force = force;
	Particles[piIndex] = pi;
}