#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint piIndex = DispatchThreadID[0];

	if (piIndex >= particlesNum)
		return;

	Particles[piIndex].velocity += Particles[piIndex].acceleration * deltaTime;
}