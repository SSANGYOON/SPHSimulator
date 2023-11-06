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
#include "Mesh.h"
#include "Obstacle.h"

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
    cubeMap->Load(L"Texture/SaintPetersBasilica.dds");
    InitParticles();
}

SPHSystem::~SPHSystem()
{
    delete[](particles);
}


void SPHSystem::InitParticles()
{
    Intances = make_unique<InstancingBuffer>();
    Intances->Init(MaxParticle);
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
    particleBuffer->Create(sizeof(Particle), MaxParticle, particles,true, false);

    hashToParticleIndexTable = make_unique<StructuredBuffer>();
    hashToParticleIndexTable->Create(sizeof(UINT), MaxParticle, nullptr, true, false);

    IndirectGPU = make_unique<StructuredBuffer>();
    IndirectGPU->Create(sizeof(IndirectArgs), 1, nullptr, true, false);

    ParticleIndirect = make_unique<IndirectBuffer>(1, sizeof(IndirectArgs), nullptr);

    ParticleWorldMatrixes = make_unique<StructuredBuffer>();
    ParticleWorldMatrixes->Create(sizeof(Vector3), MaxParticle, nullptr, true, false);


    /*
    텍스쳐 해상도는 나중에 조절
    */

    WindowInfo Info = GEngine->GetWindow();

    SceneFrontDepth = make_unique<Texture>();
    SceneFrontDepth->Create(Info.width, Info.height, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    SceneBackwardDepth = make_unique<Texture>();
    SceneBackwardDepth->Create(Info.width, Info.height, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    horizontalBlurredFrontDepth = make_unique<Texture>();
    horizontalBlurredFrontDepth->Create(Info.width, Info.height, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    horizontalBlurredBackwardDepth = make_unique<Texture>();
    horizontalBlurredBackwardDepth->Create(Info.width, Info.height, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    normalMap = make_unique<Texture>();
    normalMap->Create(Info.width, Info.height, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    backgroundTexture = make_unique<Texture>();
    backgroundTexture->Create(Info.width, Info.height, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET
        | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, 0);

    obstacle = make_unique<Mesh>();

    obstacle->Load(L"Mesh/littleDolphin.obj");

    so = make_unique<Obstacle>();
    so->SetName("Obstacle");
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
    UINT TableSize = NextPowerOf2(particleCount);


    UINT groups = particleCount % 256 > 0 ? ((particleCount >> 8) + 1) : (particleCount >> 8);
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
    pcb.tableSize = TableSize;

    auto particleCBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLE);

    particleCBuffer->SetData(&pcb);
    particleCBuffer->SetPipline(ShaderStage::CS);
    particleCBuffer->SetPipline(ShaderStage::VS);
    particleCBuffer->SetPipline(ShaderStage::PS);

    auto CalculateHashShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CalculateHashShader");
    CalculateHashShader->SetThreadGroups(TableSize >> 8, 1, 1);
    particleBuffer->BindUAV(0);
    hashToParticleIndexTable->BindUAV(1);
    CalculateHashShader->Dispatch();
    
    auto BitonicSortShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"BitonicSortShader");
    BitonicSortShader->SetThreadGroups(TableSize >> 8, 1, 1);

    auto particleSortBuffer = GEngine->GetConstantBuffer(Constantbuffer_Type::PARTICLESORT);
    
    
    for (uint32_t k = 2; k <= TableSize; k <<= 1) {
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
    WindowInfo Info = GEngine->GetWindow();

    D3D11_VIEWPORT _viewPort = { 0.0f, 0.0f, (float)Info.width, (float)Info.height, 0.0f, 1.0f };
    ID3D11DepthStencilView* commonDepth = GEngine->GetCommonDepth();

    float farClip = Cam->GetFarClip();
    float nearClip = Cam->GetNearClip();
    float zero[4] = {};

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
    matCB.viewPort = Vector2((float)Info.width, (float)Info.height);


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

    //배경 텍스쳐 바인딩
    CONTEXT->ClearRenderTargetView(backgroundTexture->GetRTV(), zero);
    CONTEXT->ClearDepthStencilView(commonDepth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    CONTEXT->OMSetRenderTargets(1, backgroundTexture->GetRTVRef(), commonDepth);

    CONTEXT->RSSetViewports(1, &_viewPort);

    //Blur
    SceneBackwardDepth->BindUAV(0);
    horizontalBlurredBackwardDepth->BindUAV(1);
    SceneFrontDepth->BindUAV(2);
    horizontalBlurredFrontDepth->BindUAV(3);

    

    for (int i = 0; i < 2; i++) {
        //HorizontalBlur
        auto HorizontalBilateralFilter = GET_SINGLE(Resources)->Find<ComputeShader>(L"HorizontalBilateralFilter");

        HorizontalBilateralFilter->SetThreadGroups(Info.width / 16, Info.height  / 16, 1);
        HorizontalBilateralFilter->Dispatch();

        //VerticalBlur
        auto VerticalBilateralFilter = GET_SINGLE(Resources)->Find<ComputeShader>(L"VerticalBilateralFilter");

        VerticalBilateralFilter->SetThreadGroups(Info.width / 16, Info.height / 16, 1);
        VerticalBilateralFilter->Dispatch();
    }

    SceneBackwardDepth->ClearUAV(0);
    horizontalBlurredBackwardDepth->ClearUAV(1);
    SceneFrontDepth->ClearUAV(2);
    horizontalBlurredFrontDepth->ClearUAV(3);;

    //Getnormal from depth
    SceneFrontDepth->BindUAV(0);
    normalMap->BindUAV(1);

    auto createNormal = GET_SINGLE(Resources)->Find<ComputeShader>(L"createNormal");

    createNormal->SetThreadGroups(Info.width / 16, Info.height / 16, 1);
    createNormal->Dispatch();

    normalMap->ClearUAV(1);
    SceneFrontDepth->ClearUAV(0);

    //배경 렌더링
    cubeMap->BindSRV(ShaderStage::PS, 0);

    auto backgroundShader = GET_SINGLE(Resources)->Find<Shader>(L"DrawBackground");
    backgroundShader->BindShader();
    RectMesh->BindBuffer();
    RectMesh->Render();

    cubeMap->ClearSRV(ShaderStage::PS, 0);

    //장애물 렌더링(임시)

    auto meshRenderingShader = GET_SINGLE(Resources)->Find<Shader>(L"HardCoded3DShader");

    trCB.world = Matrix::Identity * 0.1f;
    trCB.world._44 = 1.f;
    trCB.view = Cam->GetViewMatrix();
    trCB.projection = Cam->GetProjectionMatrix();
    trCB.viewInv = trCB.view.Invert();
    trCB.projectionInv = trCB.projection.Invert();

    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);
    cb->SetPipline(ShaderStage::PS);
    meshRenderingShader->BindShader();
    obstacle->BindBuffer();
    obstacle->Render();

    //유체 렌더링

    GEngine->BindSwapChain();
    GEngine->ClearSwapChain(true);

    SceneFrontDepth->BindSRV(ShaderStage::PS, 0);
    SceneBackwardDepth->BindSRV(ShaderStage::PS, 1);
    normalMap->BindSRV(ShaderStage::PS, 2);
    cubeMap->BindSRV(ShaderStage::PS, 3);
    backgroundTexture->BindSRV(ShaderStage::PS, 4);

    auto CompositeShader = GET_SINGLE(Resources)->Find<Shader>(L"Composite");
    CompositeShader->BindShader();
    RectMesh->BindBuffer();
    RectMesh->Render();

    SceneFrontDepth->ClearSRV(ShaderStage::PS, 0);
    SceneBackwardDepth->ClearSRV(ShaderStage::PS, 1);
    normalMap->ClearSRV(ShaderStage::PS, 2);
    cubeMap->ClearSRV(ShaderStage::PS, 3);
    backgroundTexture->ClearSRV(ShaderStage::PS, 4);

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
    ImGui::Begin("Fluid Rendering Option");

    ImGui::DragInt("SpacialFilterSize", &filterRadius, 1, 30.f);
    ImGui::ColorEdit3("SpecularColor", reinterpret_cast<float*>(&SpecularColor));
    ImGui::DragFloat("SpecularPower", &SpecularPower, 1.f, 100.f);
    ImGui::DragFloat("AbsortionCoeff", &absorbanceCoff,0.1, 80.f);
    ImGui::ColorEdit3("FluidColor", reinterpret_cast<float*>(&FluidColor));
    ImGui::DragFloat3("BoundarySize", reinterpret_cast<float*>(&boundarySize));


    static int selected = 0;
    auto pos = ImGui::GetCursorPos();
    // selectable list
    for (int n = 0; n < 10; n++)
    {
        ImGui::PushID(n);

        char buf[32];
        sprintf_s(buf, "##Object %d", n);
        

        ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
        if (ImGui::Selectable(buf, n == selected, 0, ImVec2(25, 50))) {
            selected = n;
        }
        ImGui::SetItemAllowOverlap();

        ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
        ImGui::Text("foo");

        ImGui::SetCursorPos(ImVec2(pos.x + 30, pos.y + 5));
        if (ImGui::Button("do thing", ImVec2(70, 30)))
        {
            ImGui::OpenPopup("Setup?");
            selected = n;
            printf("SETUP CLICKED %d\n", n);
        }

        if (ImGui::BeginPopupModal("Setup?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGuiContext& g = *ImGui::GetCurrentContext();

            ImGui::Text("Setup Popup");
            if (ImGui::Button("OK", ImVec2(120, 0))) { printf("OK PRESSED!\n"); ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        ImGui::SetCursorPos(ImVec2(pos.x, pos.y + 20));
        ImGui::Text("bar");

        pos.y += 55;

        ImGui::PopID();
    }

    ImGui::End();
}

void SPHSystem::reset() {
    InitParticles();
    started = false;
}

void SPHSystem::startSimulation() {
    started = true;
}
