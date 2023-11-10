#include "Global_SPH.hlsli"

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint DId = DispatchThreadID[0];
    
    //calcultate fluid particle density and pressure
    if (DId < particlesNum)
    {
        Particle pi = Particles[DId];

        int3 cell = (pi.position / radius + 0.5f * boundarySize);
        uint piIndex = DId;
        float pDensity = 0.f;
        float h2 = radius * radius;

        //From fluid to fluid
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
                            pDensity += mass * cubicspline(dist / radius);
                        }

                        pjIndex++;
                    }
                }
            }
        }

        //From boundary to fluid
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
                        if (dist < radius) {
                            pDensity += mass * cubicspline(dist / radius);
                        }

                        pjIndex++;
                    }
                }
            }
        }

        pi.density = pDensity;

        // Calculate pressure
        float pPressure = gasConstant * (pow(pi.density / restDensity, 7.0f) - 1.0f);
        pi.pressure = pPressure;
        
        Particles[DId] = pi;
    }

    //calcultate boundary particle density and pressure
    if (DId < boundaryParticlesNum)
    {
        Particle pi = boundaryParticles[DId];
        int3 cell = (pi.position / radius + 0.5f * boundarySize);
        uint piIndex = DId;
        float pDensity = 0.f;
        float h2 = radius * radius;

        //From fluid to boundary
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
                            pDensity += mass * cubicspline(dist / radius);
                        }

                        pjIndex++;
                    }
                }
            }
        }

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
                        if (dist < radius) {
                            pDensity += mass * cubicspline(dist / radius);
                        }

                        pjIndex++;
                    }
                }
            }
        }

        pi.density = pDensity;

        // Calculate pressure
        float pPressure = gasConstant * (pow(pi.density / restDensity, 7.0f) - 1.0f);
        pi.pressure = pPressure;

        boundaryParticles[DId] = pi;
    }
}