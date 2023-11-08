#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid[0];
    boundaryNeighborTable[i] = NO_PARTICLE;
    boundaryParticles[i].hash = (i < boundaryParticlesNum) ? GetHashValueOfLocation(boundaryParticles[i].position) : NO_PARTICLE;
}