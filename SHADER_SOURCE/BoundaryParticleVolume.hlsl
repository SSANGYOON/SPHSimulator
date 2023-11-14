#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint DId = DispatchThreadID[0];

    if (DId < boundaryParticlesNum)
    {
        Particle pi = boundaryParticles[DId];
        int3 cell = int3((pi.position + boundarySize * 0.5f) / radius);
        uint piIndex = DId;
        float pDensity = 0.f;

        //From boundary to boundary
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    uint cellHash = GetHash(cell + int3(x, y, z));
                    uint pjIndex = boundaryNeighborTable[cellHash];
                    if (pjIndex == NO_PARTICLE) {
                        continue;
                    }
                    while (pjIndex < boundaryParticlesNum) {
                        Particle pj = boundaryParticles[pjIndex];
                        if (pj.hash != cellHash) {
                            break;
                        }
                        float dist = length(pj.position - pi.position);
                        if (dist < radius && dist > 1e-3f) {
                            pDensity += cubic_spline_kernel(dist);
                        }

                        pjIndex++;
                    }
                }
            }
        }

        pi.density = pDensity;

        boundaryParticles[DId] = pi;
    }
}