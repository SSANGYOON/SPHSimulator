const static uint GroupThreadNum = 1024;

RWStructuredBuffer<uint> randomNumbersToSort : register(u0);
RWStructuredBuffer<uint> prefixSum : register(u1);
RWStructuredBuffer<uint> groupSum : register(u2);
RWStructuredBuffer<uint> countingBuffer : register(u3);
RWStructuredBuffer<uint> sortedResult : register(u4);

groupshared uint LocalPrefixSum[GroupThreadNum];


uint ExclusiveScan(uint gIdx, uint gId)
{

	uint jump = 1;
	uint n = GroupThreadNum >> 1;
	for (n = GroupThreadNum >> 1; n > 0; jump <<= 1, n >>= 1)
	{
		if (gIdx < n)
		{
			uint fromInd = jump * (2 * gIdx + 1) - 1;
			uint toInd = jump * (2 * gIdx + 2) - 1;
			LocalPrefixSum[toInd] += LocalPrefixSum[fromInd];
		}
		GroupMemoryBarrierWithGroupSync();
	}

	uint GroupSum = 0;
	if (gIdx == 0)
	{
		GroupSum = LocalPrefixSum[GroupThreadNum - 1];
		LocalPrefixSum[GroupThreadNum - 1] = 0;
	}

	GroupMemoryBarrierWithGroupSync();
	jump >>= 1;
	for (n = 1; n < GroupThreadNum; jump >>= 1, n <<= 1)
	{
		if (gIdx < n)
		{
			uint fromInd = jump * (2 * gIdx + 1) - 1;
			uint toInd = jump * (2 * gIdx + 2) - 1;

			uint temp = LocalPrefixSum[toInd];
			LocalPrefixSum[toInd] += LocalPrefixSum[fromInd];
			LocalPrefixSum[fromInd] = temp;
		}
		GroupMemoryBarrierWithGroupSync();
	}

	return GroupSum;
}