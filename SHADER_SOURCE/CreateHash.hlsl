#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid[0];
    neighborTable[i] = NO_PARTICLE;
    Particles[i].hash = (i < particlesNum) ? GetHashValueOfLocation(Particles[i].position) : NO_PARTICLE;
}