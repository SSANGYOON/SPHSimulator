#include "Global_SPH.hlsli"

RWStructuredBuffer<float> target : register(u6);
groupshared float groupSum[256];

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID)
{
	uint gid = GroupThreadId.x;
	
	if (DispatchThreadID.x < particlesNum)
		groupSum[gid] = target[DispatchThreadID.x];

	// hard-coded for 256 threads
	GroupMemoryBarrierWithGroupSync();

	if (gid < 128)
		groupSum[gid] += groupSum[gid + 128];
	GroupMemoryBarrierWithGroupSync();

	if (gid < 64)
		groupSum[gid] += groupSum[gid + 64];
	GroupMemoryBarrierWithGroupSync();

	if (gid < 32)	groupSum[gid] += groupSum[gid + 32];
	if (gid < 16)	groupSum[gid] += groupSum[gid + 16];
	if (gid < 8)	groupSum[gid] += groupSum[gid + 8];
	if (gid < 4)	groupSum[gid] += groupSum[gid + 4];
	if (gid < 2)	groupSum[gid] += groupSum[gid + 2];
	if (gid < 1)	groupSum[gid] += groupSum[gid + 1];

	// Have the first thread write out to the output
	if (gid == 0)
		target[DispatchThreadID.x] = groupSum[0];
}