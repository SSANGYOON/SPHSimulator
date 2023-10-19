#include "pch.h"

#include "SPHSystem.h"
#include "Mesh.h"
#include <cmath>
#include "StructuredBuffer.h"
#include "Resources.h"
#include "ComputeShader.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "Camera.h"
#include "InstancingBuffer.h"

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
    poly6 = 315.0f / (64.0f * XM_PI * pow(h, 9));
    spikyGrad = -45.0f / (XM_PI * pow(h, 6));
    spikyLap = 45.0f / (XM_PI * pow(h, 6));
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
    particles = new Particle[MaxParticle];

    started = false;

    //sphere = new Geometry("resources/lowsphere.obj");
    sphereModelMtxs = new Matrix[MaxParticle];

    Cam = make_unique<Camera>();
    Cam->Update();

    InitParticles();
}

void SPHSystem::InitParticles()
{
    Intances = make_unique<InstancingBuffer>();

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
                    k * particleSeperation + ranZ + 1.5f);

                size_t particleIndex
                    = i + (j + particleCubeWidth * k) * particleCubeWidth;
                Particle* particle = &particles[particleIndex];
                particle->position = nParticlePos;
                particle->velocity = Vector3::Zero;

                sphereModelMtxs[particleIndex]
                    = Matrix::CreateScale(settings.h, settings.h, settings.h) * Matrix::CreateTranslation(particle->position);

                Intances->AddData(sphereModelMtxs[particleIndex]);
            }
        }
    }

    particleBuffer = make_unique<StructuredBuffer>();
    particleBuffer->Create(sizeof(Particle), 4096, particles,true, true);

    hashcountedBuffer = make_unique<StructuredBuffer>();
    hashcountedBuffer->Create(sizeof(UINT), 4096, nullptr, true, false);

    offsetBuffer = make_unique<StructuredBuffer>();
    offsetBuffer->Create(sizeof(UINT), 4096, nullptr, true, false);

    prefixSumBuffer = make_unique<StructuredBuffer>();
    prefixSumBuffer->Create(sizeof(UINT), 4096, nullptr, true, false);

    groupSumBuffer = make_unique<StructuredBuffer>();
    groupSumBuffer->Create(sizeof(UINT), 1024, nullptr, true, false);

    sortedResultBuffer = make_unique<StructuredBuffer>();
    sortedResultBuffer->Create(sizeof(Particle), 4096, nullptr, true, true);

    hashToParticleIndexTable = make_unique<StructuredBuffer>();
    hashToParticleIndexTable->Create(sizeof(UINT), 4096, nullptr, true, false);
}

UINT SPHSystem::GetHashFromCell(int x, int y, int z)
{
    return (UINT)((x * 73856093) ^ (y * 19349663) ^ (x * 83492791)) % 4093;
}

UINT SPHSystem::GetHashOnCPU(Particle& p)
{
    Vector3 cell = p.position / settings.h;

    return (UINT)(((int)cell.x * 73856093) ^ ((int)cell.y * 19349663) ^ ((int)cell.x * 83492791)) % 4093;
}

void SPHSystem::CalculatePressreAndDensityOnCpu()
{
    const UINT NO_PARTICLE = 0xFFFFFFFF;
    float massPoly6Product = settings.mass * settings.poly6;

    for (size_t piIndex = 0; piIndex < 4036; piIndex++) {
        float pDensity = 0;
        Particle* pi = &CPUSortedParticle[piIndex];
        Vector3 cell = pi->position / settings.h;

        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    uint32_t cellHash = GetHashFromCell((int)cell.x + x, (int)cell.y + y, (int)cell.z + z);
                    uint32_t pjIndex = CPUSortedParticle[cellHash].hash;
                    if (pjIndex == NO_PARTICLE) {
                        continue;
                    }
                    while (pjIndex < particleCount) {
                        if (pjIndex == piIndex) {
                            pjIndex++;
                            continue;
                        }
                        Particle* pj = &particles[pjIndex];
                        if (pj->hash != cellHash) {
                            break;
                        }
                        float dist2 = (pj->position - pi->position).LengthSquared();
                        if (dist2 < settings.h2) {
                            pDensity += massPoly6Product
                                * (settings.h2 - dist2, 3) * (settings.h2 - dist2, 3) * (settings.h2 - dist2, 3);
                        }
                        pjIndex++;
                    }
                }
            }
        }

        // Include self density (as itself isn't included in neighbour)
        pi->density = pDensity + settings.selfDens;

        // Calculate pressure
        float pPressure
            = settings.gasConstant * (pi->density - settings.restDensity);
        pi->pressure = pPressure;
    }
}

SPHSystem::~SPHSystem()
{
    delete[](particles);
    delete[](sphereModelMtxs);
}

void SPHSystem::update(float deltaTime)
{
    //if (!started) return;
    // To increase system stability, a fixed deltaTime is set
    deltaTime = 0.003;
    updateParticles(sphereModelMtxs, deltaTime);
}

void SPHSystem::updateParticles(Matrix* sphereModelMtxs, float deltaTime)
{
    UINT groups = particleCount % 1024 > 0 ? ((particleCount >> 10) + 1) : (particleCount >> 10);
    ParticleCB pcb = {};
    pcb.particlesNum = particleCount;
    pcb.radius = settings.h;
    pcb.massPoly6Product = settings.massPoly6Product;
    pcb.selfDens = settings.selfDens;
    pcb.gasConstant = settings.gasConstant;
    pcb.restDensity = settings.restDensity;
    pcb.mass = settings.mass;
    pcb.spikyGrad = settings.spikyGrad;
    pcb.spikyLap = settings.spikyLap;
    pcb.viscosity = settings.viscosity;
    pcb.gravity = settings.g;
    pcb.deltaTime = deltaTime;

    auto particleCBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLE);

    particleCBuffer->SetData(&pcb);
    particleCBuffer->SetPipline(ShaderStage::CS);

    //Set up hashes for all particles
    auto CountShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CountingShader");
    CountShader->SetThreadGroups(groups, 1, 1);
    particleBuffer->BindUAV(0);
    hashcountedBuffer->BindUAV(1);
    offsetBuffer->BindUAV(2);
    CountShader->Dispatch();

    //Sort particle by hash
    auto prefixSumOnGroupShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"PrefixSumOnThreadGroupShader");
    prefixSumOnGroupShader->SetThreadGroups(groups, 1, 1);
    prefixSumBuffer->BindUAV(3);
    groupSumBuffer->BindUAV(4);
    prefixSumOnGroupShader->Dispatch();

    auto prefixSumOnGroupSumShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"PrefixSumOnGroupSumShader");
    prefixSumOnGroupSumShader->SetThreadGroups(1, 1, 1);
    prefixSumOnGroupSumShader->Dispatch();

    auto prefixCompleteShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"PrefixSumCompleteShader");
    prefixCompleteShader->SetThreadGroups(groups, 1, 1);
    prefixCompleteShader->Dispatch();

    auto countingSortCompleteShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CompleteCountingSort");
    countingSortCompleteShader->SetThreadGroups(groups, 1, 1);
    sortedResultBuffer->BindUAV(5);
    countingSortCompleteShader->Dispatch();

    //particleBuffer->Clear();
    hashcountedBuffer->Clear();
    offsetBuffer->Clear();
    prefixSumBuffer->Clear();
    groupSumBuffer->Clear();

    auto createNeighborTableShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CreateNeighborTable");
    createNeighborTableShader->SetThreadGroups(groups, 1, 1);
    hashToParticleIndexTable->BindUAV(6);
    createNeighborTableShader->Dispatch();

    auto calculatePressureAndDensityShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CalculatePressureAndDensity");
    calculatePressureAndDensityShader->SetThreadGroups(groups, 1, 1);
    calculatePressureAndDensityShader->Dispatch();
    sortedResultBuffer->GetData(GPUSortedParticle);
    auto UpdateParticlePosition = GET_SINGLE(Resources)->Find<ComputeShader>(L"UpdateParticlePosition");
    UpdateParticlePosition->SetThreadGroups(groups, 1, 1);
    UpdateParticlePosition->Dispatch();

    hashToParticleIndexTable->Clear();
    sortedResultBuffer->Clear();
    particleBuffer->Clear();

    particleBuffer->GetData(GPUSortedParticle);
    //sphereModelMtxs
    int a = 0;
}

void SPHSystem::draw()
{
    TransformCB trCB;

    trCB.world = Matrix::Identity;
    trCB.view = Matrix::Identity;
    trCB.projection = Cam->GetViewProjectionMatrix();

    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::TRANSFORM);
    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);

    auto shader = GET_SINGLE(Resources)->Find<Shader>(L"HardCoded3DShader");
    auto Lcosahedron = GET_SINGLE(Resources)->Find<Mesh>(L"Lcosahedron");
    shader->BindShader();

    Intances->Clear();
    for (int i = 0; i < 4093; i++)
    {
        Matrix mat = Matrix::CreateScale(settings.h, settings.h, settings.h) * Matrix::CreateTranslation(GPUSortedParticle[i].position);
        Intances->AddData(mat);
    }
    
    Lcosahedron->RenderInstanced(Intances.get());
}

void SPHSystem::reset() {
    InitParticles();
    started = false;
}

void SPHSystem::startSimulation() {
    started = true;
}

void SPHSystem::ResizeRatio(float width, float height)
{
    Cam->SetAspect(width / height);
}
