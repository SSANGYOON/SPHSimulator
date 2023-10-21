#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint DId = DispatchThreadID[0];
    
    Particle p;
    if (DId < particlesNum)
    {
        Particle pi = Particles[DId];

        int3 cell = pi.position / radius;

        uint piIndex = DId;

        float pDensity = 0.f;

        float h2 = radius * radius;
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    uint cellHash = GetHash(cell + int3(x, y, z));
                    uint pjIndex = neighborTable[cellHash];
                    if (pjIndex == NO_PARTICLE) {
                        continue;
                    }
                    while (pjIndex < particlesNum) {
                        if (pjIndex == piIndex) {
                            pjIndex++;
                            continue;
                        }
                        Particle pj = Particles[pjIndex];
                        if (pj.hash != cellHash) {
                            break;
                        }
                        float3 diff = (pj.position - pi.position);
                        float dist2 = dot(diff, diff);
                        if (dist2 < h2) {
                            pDensity += massPoly6Product
                                * pow(h2 - dist2, 3);
                        }
                        pjIndex++;
                    }
                }
            }
        }

        // Include self density (as itself isn't included in neighbour)
        pi.density = pDensity + selfDens;

        // Calculate pressure
        float pPressure = gasConstant * (pi.density - restDensity);
        pi.pressure = pPressure;
        
        Particles[DId] = pi;
    }
    else
    {
        return;
    }
}