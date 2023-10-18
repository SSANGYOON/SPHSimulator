#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID, uint3 GroupId : SV_GroupID)
{
	if (DispatchThreadID[0] < TABLESIZE)
	{
		prefixSum[DispatchThreadID[0]] += groupSum[GroupId[0]];
	}
}