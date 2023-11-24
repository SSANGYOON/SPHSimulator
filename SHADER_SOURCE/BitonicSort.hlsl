#include "Global_SPH.hlsli"

void BitonicSortStep(uint i, uint j, uint k)
{
    uint l = i ^ j;

    Particle pi = Particles[i];
    Particle pl = Particles[l];
    if (l > i)
    {
        if (((i & k) == 0) && (pi.hash > pl.hash) ||
            ((i & k) != 0) && (pi.hash < pl.hash))
        {
            Particle temp = pi;
            Particles[i] = pl;
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