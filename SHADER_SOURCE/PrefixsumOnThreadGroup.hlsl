#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	uint dIdx = DispatchThreadID[0];
	uint gIdx = GroupThreadID[0];

	//TODO
	// 나중에 2048이 아니라 상수버퍼에서 파티클 갯수 읽어오기
	LocalPrefixSum[gIdx] = dIdx < 2048 ? countingBuffer[dIdx] : 0;

	uint gId = GroupId[0];
	GroupMemoryBarrierWithGroupSync();
	uint GroupSum = ExclusiveScan(gIdx, gId);

	//TODO
	// 나중에 2048이 아니라 상수버퍼에서 파티클 갯수 읽어오기
	if (dIdx < 2048)
	{
		prefixSum[dIdx] = LocalPrefixSum[gIdx];
	}
	if (gIdx == 0)
	{
		groupSum[gId] = GroupSum;
	}
}