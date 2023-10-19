#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN( uint3 DispatchThreadID : SV_DispatchThreadID )
{
    uint DTId = DispatchThreadID[0];
    neighborTable[DTId] = NO_PARTICLE;

    AllMemoryBarrierWithGroupSync();
    if (DTId < TABLESIZE)
    {
        uint prevHash = DTId == 0 ? NO_PARTICLE : sortedResult[DTId - 1].hash;
        uint currentHash = sortedResult[DTId].hash;

        if (currentHash != prevHash)
        {
            neighborTable[currentHash] = DTId;
        }
    }
}