#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid[0];
    if (i < particlesNum)
    {
        uint prevHash = i == 0 ? NO_PARTICLE : Particles[i - 1].hash;
        uint currentHash = Particles[i].hash;

        if (currentHash != prevHash && currentHash != NO_PARTICLE)
        {
            neighborTable[currentHash] = i;
        }
    }
}