#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	uint dIdx = DispatchThreadID[0];
	uint gIdx = GroupThreadID[0];

	//TODO
	//고정 상수인 TABLESIZE가 아니라 상수버퍼에서 읽기
	LocalPrefixSum[gIdx] = dIdx < TABLESIZE ? CountedHash[dIdx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	uint GroupSum = ExclusiveScan(gIdx, gId);

	//TODO
	//고정 상수인 TABLESIZE가 아니라 상수버퍼에서 읽기
	if (dIdx < TABLESIZE)
	{
		prefixSum[dIdx] = LocalPrefixSum[gIdx];
	}
	if (gIdx == 0)
	{
		groupSum[gId] = GroupSum;
	}
}