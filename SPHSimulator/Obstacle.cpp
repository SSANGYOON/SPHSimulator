#include "pch.h"
#include "Obstacle.h"
#include "Camera.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "Resources.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "SDF.h"
#include "Mesh.h"
#include "StructuredBuffer.h"

Obstacle::Obstacle()
{
}

Obstacle::~Obstacle()
{
}

void Obstacle::Render(Camera* Cam)
{
    auto meshRenderingShader = GET_SINGLE(Resources)->Find<Shader>(L"HardCoded3DShader");
    
    TransformCB trCB;

    trCB.world = srt;
    trCB.view = Cam->GetViewMatrix();
    trCB.projection = Cam->GetProjectionMatrix();
    trCB.viewInv = trCB.view.Invert();
    trCB.projectionInv = trCB.projection.Invert();

    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::TRANSFORM);
    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);
    cb->SetPipline(ShaderStage::PS);
    cb->SetPipline(ShaderStage::CS);

    cb->SetData(&trCB);
    cb->SetPipline(ShaderStage::VS);
    cb->SetPipline(ShaderStage::PS);
    meshRenderingShader->BindShader();
    obstacleMesh->BindBuffer();
    obstacleMesh->Render();
}

void Obstacle::ComputeVolumeMap(float h)
{
    SDF::build(obstacleMesh, sdf, scale, h);

    sdfBuffer = make_unique<StructuredBuffer>();
    auto sizexyz = sdf.size();
    UINT size = get<0>(sizexyz) * get<1>(sizexyz) * get<2>(sizexyz);
    sdfBuffer->Create(sizeof(float), size, (void*)sdf.data(), true, false);

    volumeBuffer = make_unique<StructuredBuffer>();
    volumeBuffer->Create(sizeof(float), size, nullptr, true, true);

    SDFPropertyCB sdfProperty;

    sdfProperty.X = get<0>(sizexyz);
    sdfProperty.Y = get<1>(sizexyz);
    sdfProperty.Z = get<2>(sizexyz);

    sdfProperty.cellSize = h;

    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::SDFPROPERTY);
    cb->SetData(&sdfProperty);
    cb->SetPipline(ShaderStage::CS);

    sdfBuffer->BindUAV(0);
    volumeBuffer->BindUAV(1);

    auto volumeMapShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"ComputeVolumeMap");

    UINT groupNumX = (get<0>(sizexyz) % 8 > 0) ? get<0>(sizexyz) / 8 + 1 : get<0>(sizexyz) / 8;
    UINT groupNumY = (get<1>(sizexyz) % 8 > 0) ? get<1>(sizexyz) / 8 + 1 : get<1>(sizexyz) / 8;
    UINT groupNumZ = (get<2>(sizexyz) % 8 > 0) ? get<2>(sizexyz) / 8 + 1 : get<2>(sizexyz) / 8;

    volumeMapShader->SetThreadGroups(groupNumX, groupNumY, groupNumZ);

    volumeMapShader->Dispatch();

    vector<float> volumeVector(size);
    volumeBuffer->GetData(volumeVector.data());
}

void Obstacle::BindObstacleBuffer()
{
    ObstacleCB obstacleCB;
    obstacleCB.origin = position;
    obstacleCB.offset = sdf.origin();
    obstacleCB.size = sdf.size();
    
    shared_ptr<ConstantBuffer> cb = GEngine->GetConstantBuffer(Constantbuffer_Type::OBSTACLE);
    cb->SetData(&obstacleCB);
    cb->SetPipline(ShaderStage::CS);

    sdfBuffer->BindUAV(4);
    volumeBuffer->BindUAV(5);
}

void Obstacle::ClearObstacleBuffer()
{
    sdfBuffer->Clear();
    volumeBuffer->Clear();
}
