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
#include "IndirectBuffer.h"
#include "Texture.h"

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
  
    InitParticles();

    SceneDepth = make_unique<Texture>();
    SceneDepth->Create(1920, 1080, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
    | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

}

void SPHSystem::InitParticles()
{
    Intances = make_unique<InstancingBuffer>();
    Intances->Init(32768);
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
            }
        }
    }

    particleBuffer = make_unique<StructuredBuffer>();
    particleBuffer->Create(sizeof(Particle), 32768, particles,true, false);

    hashToParticleIndexTable = make_unique<StructuredBuffer>();
    hashToParticleIndexTable->Create(sizeof(UINT), 32768, nullptr, true, false);

    ParticleIndirect = make_unique<IndirectBuffer>(1, sizeof(IndirectArgs), nullptr);

    ParticleWorldMatrixes = make_unique<StructuredBuffer>();
    ParticleWorldMatrixes->Create(sizeof(Matrix), 32768, nullptr, true, false);
}

UINT SPHSystem::GetHashFromCell(int x, int y, int z)
{
    return (UINT)((x * 73856093) ^ (y * 19349663) ^ (z * 83492791)) % 32768;
}

UINT SPHSystem::GetHashOnCPU(Particle& p)
{
    Vector3 cell = p.position / settings.h;

    return (UINT)(((int)cell.x * 73856093) ^ ((int)cell.y * 19349663) ^ ((int)cell.z * 83492791)) % 32768;
}

SPHSystem::~SPHSystem()
{
    delete[](particles);
}

void SPHSystem::update(float deltaTime)
{
    if (!started) return;
    // To increase system stability, a fixed deltaTime is set
    //deltaTime = 0.003;
    updateParticles(deltaTime);
}

void SPHSystem::updateParticles(float deltaTime)
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

    auto CalculateHashShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CalculateHashShader");
    CalculateHashShader->SetThreadGroups(32, 1, 1);
    particleBuffer->BindUAV(0);
    hashToParticleIndexTable->BindUAV(1);
    CalculateHashShader->Dispatch();

    auto BitonicSortShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"BitonicSortShader");
    BitonicSortShader->SetThreadGroups(32, 1, 1);

    auto particleSortBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLESORT);
    //TODO 4096에서 동적인 테이블 사이즈로
    for (uint32_t k = 2; k <= 1<<15; k <<= 1){
        for (uint32_t j = k >> 1; j > 0; j >>= 1){

            ParticleSortCB pscb;
            pscb.j = j;
            pscb.k = k;

            particleSortBuffer->SetData(&pscb);
            particleSortBuffer->SetPipline(ShaderStage::CS);

            BitonicSortShader->Dispatch();
        }
    }

    auto createNeighborTableShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CreateNeighborTable");
    createNeighborTableShader->SetThreadGroups(groups, 1, 1);
    createNeighborTableShader->Dispatch();

    auto calculatePressureAndDensityShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CalculatePressureAndDensity");
    calculatePressureAndDensityShader->SetThreadGroups(groups, 1, 1);
    calculatePressureAndDensityShader->Dispatch();

    auto calculateForceShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CalculateForceShader");
    calculateForceShader->SetThreadGroups(groups, 1, 1);
    calculateForceShader->Dispatch();

    auto UpdateParticlePosition = GET_SINGLE(Resources)->Find<ComputeShader>(L"UpdateParticlePosition");
    UpdateParticlePosition->SetThreadGroups(groups, 1, 1);
    ParticleWorldMatrixes->BindUAV(3);
    ParticleIndirect->BindUAV(2);
    UpdateParticlePosition->Dispatch();

    hashToParticleIndexTable->Clear();
    particleBuffer->Clear();
    ParticleIndirect->ClearUAV();
    ParticleWorldMatrixes->Clear();
}

void SPHSystem::draw(Camera* Cam)
{
    ID3D11DepthStencilView* commonDepth = GEngine->GetCommonDepth();


    float farClip = 0.5f; //Cam->GetFarClip();

    TransformCB trCB;

    trCB.world = Matrix::Identity;
    trCB.view = Cam->GetViewMatrix();
    trCB.projection = Cam->GetProjectionMatrix();
    trCB.viewInv = trCB.view.Invert();

    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::TRANSFORM);
    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);
    cb->SetPipline(ShaderStage::PS);

    MaterialCB matCB;

    matCB.farClip = Cam->GetFarClip();
    matCB.nearClip = Cam->GetNearClip();
    matCB.viewPort = Vector2(1920, 1080);

    shared_ptr<ConstantBuffer> matcb = GEngine->GetConstantBuffer(Constantbuffer_Type::MATERIAL);
    matcb->SetData(&matCB);
    matcb->SetPipline(ShaderStage::VS);
    matcb->SetPipline(ShaderStage::PS);
    matcb->SetPipline(ShaderStage::CS);

    auto RectMesh = GET_SINGLE(Resources)->Find<Mesh>(L"RectMesh");
    Intances->SetDataFromBuffer(ParticleWorldMatrixes->GetBuffer());

    CONTEXT->ClearRenderTargetView(SceneDepth->GetRTV(), &farClip);
    CONTEXT->ClearDepthStencilView(commonDepth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    CONTEXT->OMSetRenderTargets(1, SceneDepth->GetRTVRef(), commonDepth);

    D3D11_VIEWPORT _viewPort = { 0.0f, 0.0f, 1920.f, 1080.f, 0.0f, 1.0f };

    CONTEXT->RSSetViewports(1, &_viewPort);

    auto depthRecordShader = GET_SINGLE(Resources)->Find<Shader>(L"RecordDepthShader");
    depthRecordShader->BindShader();
    RectMesh->RenderIndexedInstancedIndirect(Intances.get(), ParticleIndirect.get());

    GEngine->BindSwapChain();
    GEngine->ClearSwapChain();

    auto visualizeDepthShader = GET_SINGLE(Resources)->Find<Shader>(L"visualizeDepthShader");
    visualizeDepthShader->BindShader();
    SceneDepth->BindSRV(ShaderStage::PS, 0);
    RectMesh->BindBuffer();
    RectMesh->Render();

    SceneDepth->ClearSRV(ShaderStage::PS, 0);

    /*GEngine->ClearSwapChain();
    auto Sphereshader = GET_SINGLE(Resources)->Find<Shader>(L"HardCoded3DShader");
    
    Sphereshader->BindShader();
    RectMesh->RenderIndexedInstancedIndirect(Intances.get(), ParticleIndirect.get());*/
}

void SPHSystem::reset() {
    InitParticles();
    started = false;
}

void SPHSystem::startSimulation() {
    started = true;
}
