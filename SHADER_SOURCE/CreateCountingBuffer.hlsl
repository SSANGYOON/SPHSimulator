#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum,1,1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint DId = DispatchThreadID[0];
	CountedHash[DId] = 0;
	AllMemoryBarrierWithGroupSync();
	if (DId < particlesNum)
	{
		uint hash = GetHashValueOfLocation(Particles[DId].position);
		uint original = 0;
		InterlockedAdd(CountedHash[hash], 1, original);
		Offsets[DId] = original;
		Particles[DId].hash = hash;
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