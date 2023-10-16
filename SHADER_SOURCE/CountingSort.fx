#include "global.fx"


RWStructuredBuffer<uint> randomNumbersToSort : register(u0);
RWStructuredBuffer<uint> groupSum : register(u1);
groupshared int LocalPrefixSum[];

[numthreads(1024,1,1)]
void InsertToSharedBuffer()
{

}

[numthreads(1024, 1, 1)]
void CreateGroupSumAndExclusiveScanOnLocalBuffer()
{

}

[numthreads(1024, 1, 1)]
void CreateExclusiveSumOnBlockSum()
{
}

[numthreads(1024, 1, 1)]
void CountingSort
{
}