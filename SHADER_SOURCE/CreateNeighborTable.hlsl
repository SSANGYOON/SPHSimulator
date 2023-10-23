#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid[0];
    neighborTable[i] = NO_PARTICLE;

    AllMemoryBarrierWithGroupSync();
    if (i < TABLESIZE)
    {
        uint prevHash = i == 0 ? NO_PARTICLE : Particles[i - 1].hash;
        uint currentHash = Particles[i].hash;

        if (currentHash != prevHash)
        {
            neighborTable[currentHash] = i;
        }
    }
}