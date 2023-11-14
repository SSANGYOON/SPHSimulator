#include "pch.h"
#include "Obstacle.h"
#include "Camera.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include "Resources.h"
#include "Shader.h"
#include "VoxelGrid.h"
#include "SDF.H"
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

void Obstacle::GetVoxels(vector<Vector3>& results, float h)
{
    obstacleMesh->Voxelize(results, h, srt);

    VoxelGrid<float> sdf(int(10/0.15f) + 1, int(10 / 0.15f) + 1, int(10 / 0.15f) + 1);
}
