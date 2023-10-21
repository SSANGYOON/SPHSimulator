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
					float3 diff = pj.position - pi.position;
					float dist2 = dot(diff, diff);
					if (dist2 < h2) {
						//unit direction and length
						float dist = sqrt(dist2);
						float3 dir = normalize(diff);

						//apply pressure force
						float3 pressureForce = -dir * mass * (pi.pressure + pj.pressure) / (2 * pj.density) * spikyGrad;
						pressureForce *= pow(radius - dist, 2);
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