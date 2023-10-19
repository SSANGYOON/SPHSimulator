#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
	uint DId = DispatchThreadID[0];
	if (DId < particlesNum)
	{
		Particle r = Particles[DId];
		uint original = 0;
		sortedResult[prefixSum[r.hash] + Offsets[DId]] = r;
	}
}