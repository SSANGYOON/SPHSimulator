#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	//TODO
	// ���߿� ������TABLESIZE �ƴ϶� ������ۿ��� ��ƼŬ ���� �о����

	uint groups = TABLESIZE % 1024 > 0 ? ((TABLESIZE >> 10) + 1) : (TABLESIZE >> 10);
	uint gldx = GroupThreadID[0];
	LocalPrefixSum[gldx] = gldx < groups ? groupSum[gldx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	ExclusiveScan(gldx, gId);
	groupSum[gldx] = gldx < groups ? LocalPrefixSum[gldx] : 0;
}