#include "Global_SPH.hlsli"

RWStructuredBuffer<float> target : register(u6);
groupshared float groupSum[1024];

[numthreads(1024, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID)
{
	uint gid = GroupThreadId.x;
	
	if (DispatchThreadID.x < particlesNum)
		groupSum[gid] = target[DispatchThreadID.x];

	uint n = 512;

	[unroll]
	for (int i = 0; i < 10; i++)
	{
		GroupMemoryBarrierWithGroupSync();
		if (gid < n)
		{
			groupSum[gid] += groupSum[gid + n];
		}
		n = n >> 1;
	}

	if (gid == 0)
		target[DispatchThreadID.x] = groupSum[0];
}