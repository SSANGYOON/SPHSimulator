#include "pch.h"
#include "Obstacle.h"
#include "Camera.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "Resources.h"
#include "Shader.h"

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

    trCB.world = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
    trCB.world._44 = 1.f;
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
