#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
	if (DispatchThreadID[0] < particlesNum)
	{
		Particle r = Particles[DispatchThreadID[0]];

		uint hash = GetHashValueOfLocation(r.position);
		uint original = 0;
		InterlockedAdd(prefixSum[hash], 1, original);
		sortedResult[original] = r;
	}
}