#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];
	if (piIndex >= particlesNum)
		return;

	Particle pi = Particles[piIndex];


	float boxWidth = 4.f;
	float elasticity = 0.5f;


	//calculate acceleration and velocity
	float3 acceleration = pi.force / pi.density + float3(0, gravity, 0);
	float3 velocity = pi.velocity + acceleration * deltaTime;

	// Update position
	float3 position = pi.position + velocity * deltaTime;

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

	float4x4 mat = { radius, 0.0f, 0.0f, p.position.x,
					 0.0f, radius, 0.0f, p.position.y,
					 0.0f, 0.0f, radius, p.position.z,
					 0.0f, 0.0f, 0.0f, 1.f };

	ParticleWorld[piIndex] = mat;
}