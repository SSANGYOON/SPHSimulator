#include "pch.h"
#include "particle.h"
#include "SPHSystem.h"
#include "Mesh.h"

#define PI 3.14159265f

SPHSettings::SPHSettings(
    float mass, float restDensity, float gasConst, float viscosity, float h,
    float g, float tension)
    : mass(mass)
    , restDensity(restDensity)
    , gasConstant(gasConst)
    , viscosity(viscosity)
    , h(h)
    , g(g)
    , tension(tension)
{
    poly6 = 315.0f / (64.0f * PI * pow(h, 9));
    spikyGrad = -45.0f / (PI * pow(h, 6));
    spikyLap = 45.0f / (PI * pow(h, 6));
    h2 = h * h;
    selfDens = mass * poly6 * pow(h, 6);
    massPoly6Product = mass * poly6;
    sphereScale = Matrix::CreateScale(Vector3(h/2.f));
}

SPHSystem::SPHSystem(UINT32 particleCubeWidth, const SPHSettings& settings)
    : particleCubeWidth(particleCubeWidth)
    , settings(settings)
{
    particleCount = particleCubeWidth * particleCubeWidth * particleCubeWidth;
    particles = (Particle*)malloc(sizeof(Particle) * particleCount);

    started = false;

    //sphere = new Geometry("resources/lowsphere.obj");
    sphereModelMtxs = new Matrix[particleCount];

    InitParticles();
}

void SPHSystem::InitParticles()
{
    std::srand(1024);
    float particleSeperation = settings.h + 0.01f;
    for (int i = 0; i < particleCubeWidth; i++) {
        for (int j = 0; j < particleCubeWidth; j++) {
            for (int k = 0; k < particleCubeWidth; k++) {
                float ranX
                    = (float(rand()) / float((RAND_MAX)) * 0.5f - 1)
                    * settings.h / 10;
                float ranY
                    = (float(rand()) / float((RAND_MAX)) * 0.5f - 1)
                    * settings.h / 10;
                float ranZ
                    = (float(rand()) / float((RAND_MAX)) * 0.5f - 1)
                    * settings.h / 10;
                Vector3 nParticlePos = Vector3(
                    i * particleSeperation + ranX - 1.5f,
                    j * particleSeperation + ranY + settings.h + 0.1f,
                    k * particleSeperation + ranZ - 1.5f);

                size_t particleIndex
                    = i + (j + particleCubeWidth * k) * particleCubeWidth;
                Particle* particle = &particles[particleIndex];
                particle->position = nParticlePos;
                particle->velocity = Vector3::Zero;

                sphereModelMtxs[particleIndex]
                    = Matrix::CreateTranslation(particle->position) * settings.sphereScale;
            }
        }
    }
}

SPHSystem::~SPHSystem()
{
    delete[](particles);
}

void SPHSystem::update(float deltaTime)
{
    if (!started) return;
    // To increase system stability, a fixed deltaTime is set
    deltaTime = 0.003f;
    //updateParticles(particles, sphereModelMtxs, particleCount, settings, deltaTime);
}

void SPHSystem::draw()
{

}

void SPHSystem::reset() {
    InitParticles();
    started = false;
}

void SPHSystem::startSimulation() {
    started = true;
}