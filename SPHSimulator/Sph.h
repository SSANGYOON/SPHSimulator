#include "Particle.h"
#include "SPHSystem.h"

/// Calculates and stores particle hashes.
void parallelCalculateHashes(
    Particle* particles, UINT32 start, UINT32 end, const SPHSettings& settings);

/// Sort particles in place by hash.
void sortParticles(Particle* particles, const size_t& particleCount);

/// Update attrs of particles in place.
void updateParticles(
    Particle* particles, Matrix* particleTransforms,
    const size_t particleCount, const SPHSettings& settings,
    float deltaTime, const bool onGPU);