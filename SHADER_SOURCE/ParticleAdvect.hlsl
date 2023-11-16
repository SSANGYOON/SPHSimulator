#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];

	if (piIndex >= particlesNum)
		return;

	Particle pi = Particles[piIndex];

	float boxWidth = 5.f;
	float elasticity = 1.f;

	// Update position
	float3 velocity = pi.velocity;
	float3 position = pi.position + velocity * deltaTime;

	// Handle collisions with box
	if (position.y < radius * 0.5f + boundaryCentor.y - boundarySize.y / 2) {
		position.y = -position.y + 2 * (radius * 0.5f + boundaryCentor.y - boundarySize.y * 0.5f) + 0.0001f;
		velocity.y = -velocity.y * elasticity;
	}

	if (position.x < radius * 0.5f + boundaryCentor.x - boundarySize.x / 2) {

		position.x = -position.x + 2 * (radius * 0.5f + boundaryCentor.x - boundarySize.x * 0.5f) + 0.0001f;
		velocity.x = -velocity.x * elasticity;
	}

	if (position.x > -radius * 0.5f + boundaryCentor.x + boundarySize.x / 2) {
		position.x = -position.x + 2 * (-radius * 0.5f + boundaryCentor.x + boundarySize.x * 0.5f) - 0.0001f;
		velocity.x = -velocity.x * elasticity;
	}

	if (position.z < radius * 0.5f + boundaryCentor.z - boundarySize.z / 2) {
		position.z = -position.z + 2 * (radius * 0.5f + boundaryCentor.z - boundarySize.z * 0.5f) + 0.0001f;
		velocity.z = -velocity.z * elasticity;
	}

	if (position.z > -radius * 0.5f + boundaryCentor.z + boundarySize.z / 2) {
		position.z = -position.z + 2 * (-radius * 0.5f + boundaryCentor.z + boundarySize.z * 0.5f) - 0.0001f;
		velocity.z = -velocity.z * elasticity;
	}

	Particles[piIndex].position = position;
	Particles[piIndex].velocity = velocity;
	ParticleWorld[piIndex] = position;

	if (piIndex == 0)
	{
		IndirectArgs args = { 6, particlesNum, 0, 0, 0 };
		IndirectBuffer[0] = args;
	}
}