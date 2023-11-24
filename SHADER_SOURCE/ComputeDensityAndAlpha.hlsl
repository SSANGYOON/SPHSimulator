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
                        float3 diff = pi.position - pj.position;
                        float dist = length(diff);
                        if (dist < 2.f * radius) {
                            pDensity += mass * cubic_spline_kernel(dist);
                            if (dist > 1e-3) {
                                float3 temp = mass * cubic_spline_kernel_gradient(diff);
                                gradientSum += temp;
                                gradientProduct += dot(temp, temp);
                            }
                        }

                        pjIndex++;
                    }
                }
            }
        }
        //boundary handling
        row_major matrix rotationTranspose = transpose(obstacleRotation);
        float3 boundaryLocal = (mul(float4(pi.position - obstaclePos, 1.f), rotationTranspose).xyz  - obstacleOffset) / radius;

        if ((boundaryLocal.x > 0 && boundaryLocal.x < obstacleSize.x - 1) &&
            (boundaryLocal.y > 0 && boundaryLocal.y < obstacleSize.y - 1) &&
            (boundaryLocal.z > 0 && boundaryLocal.z < obstacleSize.z - 1))
        {
            float boundarySDF = triLinearSDF(boundaryLocal);
            float boundaryVolume = triLinearVolume(boundaryLocal);
            float3 grad = sdfGradient(boundaryLocal);

            if (length(grad) > 1e-3)
            {
                float3 normal = normalize(grad);
                normal = mul(float4(normal, 0.f), obstacleRotation).xyz;

                float3 diff = clamp(boundarySDF, 1e-3, 3 * radius) * normal;
                float dist = length(diff);

                pDensity += boundaryVolume * restDensity * cubic_spline_kernel(dist);

                float3 temp = boundaryVolume * cubic_spline_kernel_gradient(diff);
                gradientSum += temp;
                gradientProduct += dot(temp, temp);
            }
        }

        pi.density = pDensity;
        pi.alpha = 1.0f / (gradientProduct + dot(gradientSum, gradientSum));
     
        Particles[DId] = pi;
    }
}