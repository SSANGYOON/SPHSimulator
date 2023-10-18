#pragma once

#include "particle.h"
#include "SPHSystem.h"

//void parallelCalculateHashes(Particle* particles, size_t start, size_t end, const SPHSettings& settings);

//void sortParticles(Particle* particles, const size_t& particleCount);

void updateParticles(Particle* particles, Matrix* particleTransforms, const UINT32 particleCount, const SPHSettings& settings, float deltatime);

