#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	uint dIdx = DispatchThreadID[0];
	uint gIdx = GroupThreadID[0];

	//TODO
	// ���߿� 2048�� �ƴ϶� ������ۿ��� ��ƼŬ ���� �о����
	LocalPrefixSum[gIdx] = dIdx < 2048 ? countingBuffer[dIdx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	uint GroupSum = ExclusiveScan(gIdx, gId);

	//TODO
	// ���߿� 2048�� �ƴ϶� ������ۿ��� ��ƼŬ ���� �о����
	if (dIdx < 2048)
	{
		prefixSum[dIdx] = LocalPrefixSum[gIdx];
	}
	if (gIdx == 0)
	{
		groupSum[gId] = GroupSum;
	}
}