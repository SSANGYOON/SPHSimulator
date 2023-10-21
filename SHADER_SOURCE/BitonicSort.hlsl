#include "Global_SPH.hlsli"

void BitonicSortStep(uint i, uint j, uint k)
{
    uint l = i ^ j;
    if (l > i)
    {
        if (((i & k) == 0) && (Particles[i].hash > Particles[l].hash) ||
            ((i & k) != 0) && (Particles[i].hash < Particles[l].hash))
        {
            Particle temp = Particles[i];
            Particles[i] = Particles[l];
            Particles[l] = temp;
        }
    }
}

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN( uint3 DTid : SV_DispatchThreadID )
{
    uint i = DTid[0];
    BitonicSortStep(i, j, k);
}