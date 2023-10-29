#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint DId = DispatchThreadID[0];
    
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
                        Particle pj = Particles[pjIndex];
                        if (pj.hash != cellHash) {
                            break;
                        }
                        float dist = length(pj.position - pi.position);
                        if (dist < radius) {
                            pDensity += massPoly6Product * pow(h2 - dist * dist, 3);
                        }
                        
                        pjIndex++;
                    }
                }
            }
        }

        pi.density = pDensity;

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