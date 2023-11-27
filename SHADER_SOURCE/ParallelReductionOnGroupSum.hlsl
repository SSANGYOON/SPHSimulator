#include "Global_SPH.hlsli"

RWStructuredBuffer<float> target : register(u6);
groupshared float groupSum[1024];

[numthreads(1024, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID)
{
	uint gid = GroupThreadId.x;

	if (gid * 1024 < particlesNum)
		groupSum[gid] = target[gid * 256];

	// hard-coded for 1024 threads
	GroupMemoryBarrierWithGroupSync();

	if (gid < 512)
		groupSum[gid] += groupSum[gid + 512];
	GroupMemoryBarrierWithGroupSync();

	if (gid < 256)
		groupSum[gid] += groupSum[gid + 256];
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

	if (gid == 0) {
		if (groupSum[0] < restDensity * 0.01f)
		{
			target[0] = ITERATIONEND;
		}
	}
}