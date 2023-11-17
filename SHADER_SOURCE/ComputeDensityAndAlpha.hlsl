#include "Global_SPH.hlsli"

RWStructuredBuffer<float> errorBuffer : register(u6);

[numthreads(GroupThreadNum, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint DId = DispatchThreadID[0];

    //calcultate fluid particle density and pressure
    if (DId < particlesNum)
    {
        Particle pi = Particles[DId];

        int3 cell = int3((pi.position + boundarySize * 0.5f) / (2.f * radius));
        uint piIndex = DId;
        float pDensity = 0.f;

        float3 gradientSum = (float3)0.f;
        float gradientProduct = 0.f;
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
                        if (dist < 2.f * radius) {
                            pDensity += mass * cubic_spline_kernel(dist);
                            if (dist > 1e-3) {
                                float3 temp = mass * cubic_spline_kernel_gradient(pj.position - pi.position);
                                gradientSum += temp;
                                gradientProduct += dot(temp, temp);
                            }
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
                        if (dist < 2.f * radius) {
                            float boundaryParticleMass = restDensity / pj.density;
                            
                            pDensity += boundaryParticleMass * cubic_spline_kernel(dist);
                            if (dist > 1e-3) {
                                float3 temp = boundaryParticleMass * cubic_spline_kernel_gradient(pj.position - pi.position);
                                gradientSum += temp;
                                gradientProduct += dot(temp, temp);
                            }
                        }

                        pjIndex++;
                    }
                }
            }
        }

        pi.density = pDensity;
        pi.alpha = 1.0f / (gradientProduct + dot(gradientSum, gradientSum));
     
        Particles[DId] = pi;
    }
}