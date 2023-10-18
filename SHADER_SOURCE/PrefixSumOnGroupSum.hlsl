#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	//TODO
	// 나중에 고정된TABLESIZE 아니라 상수버퍼에서 파티클 갯수 읽어오기

	uint groups = TABLESIZE % 1024 > 0 ? ((TABLESIZE >> 10) + 1) : (TABLESIZE >> 10);
	uint gldx = GroupThreadID[0];
	LocalPrefixSum[gldx] = gldx < groups ? groupSum[gldx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	ExclusiveScan(gldx, gId);
	groupSum[gldx] = gldx < groups ? LocalPrefixSum[gldx] : 0;
}