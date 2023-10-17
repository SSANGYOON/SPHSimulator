#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
	if (DispatchThreadID[0] < 2048)
	{
		uint r = randomNumbersToSort[DispatchThreadID[0]];
		uint original = 0;
		InterlockedAdd(prefixSum[r], 1, original);
		sortedResult[original] = r;
	}
}