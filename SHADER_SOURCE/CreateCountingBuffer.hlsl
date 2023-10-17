#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum,1,1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
	if (DispatchThreadID[0] < 2048)
	{
		uint randomNumber = randomNumbersToSort[DispatchThreadID[0]];
		uint original;
		InterlockedAdd(countingBuffer[randomNumber], 1, original);
	}
}

/*[numthreads(1024, 1, 1)]
void CreateGroupSumAndExclusiveScanOnLocalBuffer()
{

}

[numthreads(1024, 1, 1)]
void CreateExclusiveSumOnGroupSum()
{
}

[numthreads(1024, 1, 1)]
void CountingSort
{
}
*/