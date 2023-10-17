#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	//TODO
	// ���߿� 2�� �ƴ϶� ������ۿ��� ��ƼŬ ���� �о����
	uint gldx = GroupThreadID[0];
	LocalPrefixSum[gldx] = gldx < 2 ? groupSum[gldx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	ExclusiveScan(gldx, gId);
	groupSum[gldx] = gldx < 2 ? LocalPrefixSum[gldx] : 0;
}