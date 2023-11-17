#include "Global_SPH.hlsli"

RWStructuredBuffer<float> target : register(u6);
groupshared float groupSum[256];

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID)
{
	uint gid = GroupThreadId.x;
	
	if (DispatchThreadID.x < particlesNum)
		groupSum[gid] = target[DispatchThreadID.x];

	uint n = 128;

	[unroll]
	for (int i = 0; i < 8; i++)
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