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
    
    cubeMap = make_shared<Texture>();
    cubeMap->Load(L"Texture/MSPath_specularIBL.dds");
    InitParticles();
}

SPHSystem::~SPHSystem()
{
    delete[](particles);
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
                    (i - (int)particleCubeWidth / 2) * particleSeperation + ranX,
                    (j - (int)particleCubeWidth / 2) * particleSeperation + ranY + settings.h + 0.1f,
                    (k - (int)particleCubeWidth / 2) * particleSeperation + ranZ);

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

    IndirectGPU = make_unique<StructuredBuffer>();
    IndirectGPU->Create(sizeof(IndirectArgs), 1, nullptr, true, false);

    ParticleIndirect = make_unique<IndirectBuffer>(1, sizeof(IndirectArgs), nullptr);

    ParticleWorldMatrixes = make_unique<StructuredBuffer>();
    ParticleWorldMatrixes->Create(sizeof(Vector3), 32768, nullptr, true, false);


    /*
    텍스쳐 해상도는 나중에 조절
    */

    SceneFrontDepth = make_unique<Texture>();
    SceneFrontDepth->Create(1920, 1080, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    SceneBackwardDepth = make_unique<Texture>();
    SceneBackwardDepth->Create(1920, 1080, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    horizontalBlurredFrontDepth = make_unique<Texture>();
    horizontalBlurredFrontDepth->Create(1920, 1080, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    horizontalBlurredBackwardDepth = make_unique<Texture>();
    horizontalBlurredBackwardDepth->Create(1920, 1080, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    normalMap = make_unique<Texture>();
    normalMap->Create(1920, 1080, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);
}

void SPHSystem::update(float deltaTime)
{
    if (!started) return;
    // To increase system stability, a fixed deltaTime is set
    deltaTime = 0.003;
    updateParticles(deltaTime);
}

void SPHSystem::updateParticles(float deltaTime)
{

    UINT groups = particleCount % 1024 > 0 ? ((particleCount >> 10) + 1) : (particleCount >> 10);
    ParticleCB pcb = {};
    pcb.particlesNum = particleCount;
    pcb.radius = settings.h;
    pcb.massPoly6Product = settings.massPoly6Product;
    pcb.gasConstant = settings.gasConstant;
    pcb.restDensity = settings.restDensity;
    pcb.mass = settings.mass;
    pcb.spikyGrad = settings.spikyGrad;
    pcb.spikyLap = settings.spikyLap;
    pcb.viscosity = settings.viscosity;
    pcb.gravity = settings.g;
    pcb.deltaTime = deltaTime;
    pcb.boundaryCentor = boundaryCentor;
    pcb.boundarySize = boundarySize;

    auto particleCBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLE);

    particleCBuffer->SetData(&pcb);
    particleCBuffer->SetPipline(ShaderStage::CS);
    particleCBuffer->SetPipline(ShaderStage::VS);
    particleCBuffer->SetPipline(ShaderStage::PS);

    auto CalculateHashShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CalculateHashShader");
    CalculateHashShader->SetThreadGroups(32, 1, 1);
    particleBuffer->BindUAV(0);
    hashToParticleIndexTable->BindUAV(1);
    CalculateHashShader->Dispatch();

    auto BitonicSortShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"BitonicSortShader");
    BitonicSortShader->SetThreadGroups(32, 1, 1);

    auto particleSortBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLESORT);
    //TODO 4096에서 동적인 테이블 사이즈로
    for (uint32_t k = 2; k <= 1 << 15; k <<= 1) {
        for (uint32_t j = k >> 1; j > 0; j >>= 1) {

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
    IndirectGPU->BindUAV(2);
    UpdateParticlePosition->Dispatch();

    hashToParticleIndexTable->Clear();
    particleBuffer->Clear();
    IndirectGPU->Clear();
    ParticleWorldMatrixes->Clear();

    ParticleIndirect->SetDataFromBuffer(IndirectGPU->GetBuffer());
}

void SPHSystem::draw(Camera* Cam)
{
    //TODO 나중에 해상도 따라서 조절하기
    D3D11_VIEWPORT _viewPort = { 0.0f, 0.0f, 1920.f, 1080.f, 0.0f, 1.0f };
    ID3D11DepthStencilView* commonDepth = GEngine->GetCommonDepth();

    float farClip = Cam->GetFarClip();
    float nearClip = Cam->GetNearClip();

    TransformCB trCB;

    trCB.world = Matrix::Identity;
    trCB.view = Cam->GetViewMatrix();
    trCB.projection = Cam->GetProjectionMatrix();
    trCB.viewInv = trCB.view.Invert();
    trCB.projectionInv = trCB.projection.Invert();

    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::TRANSFORM);
    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);
    cb->SetPipline(ShaderStage::PS);
    cb->SetPipline(ShaderStage::CS);

    MaterialCB matCB;

    matCB.farClip = farClip++;
    matCB.nearClip = nearClip;
    // TODO 해상도는 나중에 조절할 수 있도록 하기
    matCB.viewPort = Vector2(1920, 1080);


    shared_ptr<ConstantBuffer> matcb = GEngine->GetConstantBuffer(Constantbuffer_Type::MATERIAL);
    matcb->SetData(&matCB);
    matcb->SetPipline(ShaderStage::VS);
    matcb->SetPipline(ShaderStage::PS);
    matcb->SetPipline(ShaderStage::CS);

    ParticleRenderCB prCB;

    prCB.blurDepthFalloff = blurDepthFalloff;
    prCB.filterRadius = filterRadius;
    prCB.SpecularColor = SpecularColor;
    prCB.SpecularIntensity = SpecularIntensity;
    prCB.SpecularPower = SpecularPower;
    prCB.absorbanceCoff = absorbanceCoff;
    prCB.fluidColor = FluidColor;

    shared_ptr<ConstantBuffer> prBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLERENDER);
    prBuffer->SetData(&prCB);
    prBuffer->SetPipline(ShaderStage::VS);
    prBuffer->SetPipline(ShaderStage::PS);
    prBuffer->SetPipline(ShaderStage::CS);

    auto RectMesh = GET_SINGLE(Resources)->Find<Mesh>(L"RectMesh");
    Intances->SetDataFromBuffer(ParticleWorldMatrixes->GetBuffer());

    //Scene Front Depth Rendering;
    CONTEXT->ClearRenderTargetView(SceneFrontDepth->GetRTV(), &farClip);
    CONTEXT->ClearDepthStencilView(commonDepth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    CONTEXT->OMSetRenderTargets(1, SceneFrontDepth->GetRTVRef(), commonDepth);

    CONTEXT->RSSetViewports(1, &_viewPort);

    auto frontDepthRecordShader = GET_SINGLE(Resources)->Find<Shader>(L"RecordFrontDepthShader");
    frontDepthRecordShader->BindShader();
    RectMesh->RenderIndexedInstancedIndirect(Intances.get(), ParticleIndirect.get());

    //Scene Backward Depth Rendering;
    CONTEXT->ClearRenderTargetView(SceneBackwardDepth->GetRTV(), &nearClip);
    CONTEXT->ClearDepthStencilView(commonDepth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.f, 0);
    CONTEXT->OMSetRenderTargets(1, SceneBackwardDepth->GetRTVRef(), commonDepth);

    CONTEXT->RSSetViewports(1, &_viewPort);

    auto backwardDepthRecordShader = GET_SINGLE(Resources)->Find<Shader>(L"RecordBackwardDepthShader");
    backwardDepthRecordShader->BindShader();
    RectMesh->RenderIndexedInstancedIndirect(Intances.get(), ParticleIndirect.get());

    GEngine->BindSwapChain();
    GEngine->ClearSwapChain();

    //Blur
    SceneBackwardDepth->BindUAV(0);
    horizontalBlurredBackwardDepth->BindUAV(1);
    SceneFrontDepth->BindUAV(2);
    horizontalBlurredFrontDepth->BindUAV(3);

    //HorizontalBlur
    auto HorizontalBilateralFilter = GET_SINGLE(Resources)->Find<ComputeShader>(L"HorizontalBilateralFilter");

    //TODO 해상도 나중에 조절할 수 있도록 하기
    HorizontalBilateralFilter->SetThreadGroups(1920 / 32, 1080 / 8, 1);
    HorizontalBilateralFilter->Dispatch();

    //VerticalBlur
    auto VerticalBilateralFilter = GET_SINGLE(Resources)->Find<ComputeShader>(L"VerticalBilateralFilter");

    //TODO 해상도 나중에 조절할 수 있도록 하기
    VerticalBilateralFilter->SetThreadGroups(1920 / 32, 1080 / 8, 1);
    VerticalBilateralFilter->Dispatch();

    SceneBackwardDepth->ClearUAV(0);
    horizontalBlurredBackwardDepth->ClearUAV(1);
    SceneFrontDepth->ClearUAV(2);
    horizontalBlurredFrontDepth->ClearUAV(3);

    //Getnormal from depth
    SceneFrontDepth->BindUAV(0);
    normalMap->BindUAV(1);

    auto createNormal = GET_SINGLE(Resources)->Find<ComputeShader>(L"createNormal");

    //TODO 해상도 나중에 조절할 수 있도록 하기
    createNormal->SetThreadGroups(1920 / 32, 1080 / 8, 1);
    createNormal->Dispatch();

    normalMap->ClearUAV(1);
    SceneFrontDepth->ClearUAV(0);

    //유체 렌더링

    SceneFrontDepth->BindSRV(ShaderStage::PS, 0);
    SceneBackwardDepth->BindSRV(ShaderStage::PS, 1);
    normalMap->BindSRV(ShaderStage::PS, 2);
    cubeMap->BindSRV(ShaderStage::PS, 3);

    auto CompositeShader = GET_SINGLE(Resources)->Find<Shader>(L"Composite");
    CompositeShader->BindShader();
    RectMesh->Render();

    SceneFrontDepth->ClearSRV(ShaderStage::PS, 0);
    SceneBackwardDepth->ClearSRV(ShaderStage::PS, 1);
    normalMap->ClearSRV(ShaderStage::PS, 2);
    cubeMap->ClearSRV(ShaderStage::PS, 3);

    DrawBoundary(Cam);
}

void SPHSystem::DrawBoundary(Camera* Cam)
{
    DrawWireFrame(Vector3(boundarySize.x, boundarySize.y, 1), Quaternion::Identity, boundaryCentor + Vector3(0.f, 0.f, -boundarySize.z * 0.5f), Cam);
    DrawWireFrame(Vector3(boundarySize.x, boundarySize.y, 1), Quaternion::Identity, boundaryCentor + Vector3(0.f, 0.f, boundarySize.z * 0.5f), Cam);
    DrawWireFrame(Vector3(boundarySize.z, boundarySize.y, 1), Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), XM_PIDIV2), boundaryCentor + Vector3(boundarySize.x * 0.5f, 0.f, 0.f), Cam);
    DrawWireFrame(Vector3(boundarySize.z, boundarySize.y, 1), Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), XM_PIDIV2), boundaryCentor + Vector3(-boundarySize.x * 0.5f, 0.f, 0.f), Cam);
}

void SPHSystem::DrawWireFrame(const Vector3& size, const Quaternion& rotation, const Vector3& translation, Camera* Cam)
{
    TransformCB trCB;

    auto outlinedRect = GET_SINGLE(Resources)->Find<Mesh>(L"OutlinedRect");
    auto wireFrameRenderer = GET_SINGLE(Resources)->Find<Shader>(L"OutLineShader");

    trCB.world = Matrix::CreateScale(size) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(translation);
    trCB.view = Cam->GetViewMatrix();
    trCB.projection = Cam->GetProjectionMatrix();

    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::TRANSFORM);

    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);

    wireFrameRenderer->BindShader();
    outlinedRect->BindBuffer();
    outlinedRect->Render();
}

void SPHSystem::ImGUIRender()
{
    ImGui::DragFloat("Blur Depth falloff", &blurDepthFalloff, 0.1, 80.f);
    ImGui::DragInt("SpacialFilterSize", &filterRadius, 1, 30.f);
    ImGui::ColorEdit3("SpecularColor", reinterpret_cast<float*>(&SpecularColor));
    ImGui::DragFloat("SpecularPower", &SpecularPower, 1.f, 100.f);
    ImGui::DragFloat("AbsortionCoeff", &absorbanceCoff,0.1, 80.f);
    ImGui::ColorEdit3("FluidColor", reinterpret_cast<float*>(&FluidColor));
    ImGui::DragFloat3("BoundarySize", reinterpret_cast<float*>(&boundarySize));
}

void SPHSystem::reset() {
    InitParticles();
    started = false;
}

void SPHSystem::startSimulation() {
    started = true;
}
