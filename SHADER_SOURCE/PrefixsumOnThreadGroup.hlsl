#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	uint dIdx = DispatchThreadID[0];
	uint gIdx = GroupThreadID[0];

	//TODO
	//���� ����� TABLESIZE�� �ƴ϶� ������ۿ��� �б�
	LocalPrefixSum[gIdx] = dIdx < TABLESIZE ? CountedHash[dIdx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	uint GroupSum = ExclusiveScan(gIdx, gId);

	//TODO
	//���� ����� TABLESIZE�� �ƴ϶� ������ۿ��� �б�
	if (dIdx < TABLESIZE)
	{
		prefixSum[dIdx] = LocalPrefixSum[gIdx];
	}
	if (gIdx == 0)
	{
		groupSum[gId] = GroupSum;
	}
}