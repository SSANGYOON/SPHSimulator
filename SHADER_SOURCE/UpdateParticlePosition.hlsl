#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN( uint3 DispatchThreadID : SV_DispatchThreadID )
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum)
		return;

	Particle pi = sortedResult[piIndex];

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
					Particle pj = sortedResult[pjIndex];
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

	float boxWidth = 4.f;
	float elasticity = 0.5f;


	//calculate acceleration and velocity
	float3 acceleration = force / pi.density + float3(0, gravity, 0);
	float3 velocity = pi.velocity + acceleration * deltaTime;

	// Update position
	float3 position = pi.position +velocity * deltaTime;

	// Handle collisions with box
	if (position.y < radius) {
		position.y = -position.y + 2 * radius + 0.0001f;
		velocity.y = -velocity.y * elasticity;
	}

	if (position.x < radius - boxWidth) {
		position.x = -position.x + 2 * (radius - boxWidth) + 0.0001f;
		velocity.x = -velocity.x * elasticity;
	}

	if (position.x > -radius + boxWidth) {
		position.x = -position.x + 2 * -(radius - boxWidth) - 0.0001f;
		velocity.x = -velocity.x * elasticity;
	}

	if (position.z < radius - boxWidth) {
		position.z = -position.z + 2 * (radius - boxWidth) + 0.0001f;
		velocity.z = -velocity.z * elasticity;
	}

	if (position.z > -radius + boxWidth) {
		position.z = -position.z + 2 * -(radius - boxWidth) - 0.0001f;
		velocity.z = -velocity.z * elasticity;
	}

	Particle p = (Particle)0;
	p.position = position;
	p.velocity = velocity;
	p.hash = pi.hash;
	p.density = pi.density;
	p.pressure = pi.pressure;
	Particles[piIndex] = p;
}