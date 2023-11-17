#include "Global_SPH.hlsli"

RWStructuredBuffer<float> target : register(u6);
groupshared float groupSum[1024];

[numthreads(1024, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID)
{
	uint gid = GroupThreadId.x;

	if (gid * 1024 < particlesNum)
		groupSum[gid] = target[gid * 1024];

	uint n = tableSize >> 11;

	while(n > 0)
	{
		GroupMemoryBarrierWithGroupSync();
		if (gid < n)
		{
			groupSum[gid] += groupSum[gid + n];
		}
		n = n >> 1;
	}

	if (gid == 0) {
		if (groupSum[0] < restDensity * 0.01f)
		{
			target[0] = ITERATIONEND;
		}
	}
}