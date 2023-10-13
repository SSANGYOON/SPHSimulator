#include "pch.h"
#include "TestLayer.h"
#include "Resources.h"
#include "Shader.h"
#include "Mesh.h"

SY::TestLayer::TestLayer()
{
}

SY::TestLayer::~TestLayer()
{
}

void SY::TestLayer::OnAttach()
{
}

void SY::TestLayer::OnDetach()
{
}

void SY::TestLayer::OnUpdate(float timestep)
{
	auto shader = GET_SINGLE(Resources)->Find<Shader>(L"TestShader");
	auto rect = GET_SINGLE(Resources)->Find<Mesh>(L"RectMesh");
	
	shader->BindShader();

	rect->BindBuffer();
	rect->Render();
}

void SY::TestLayer::OnImGuiRender()
{
}

void SY::TestLayer::OnEvent(Event& e)
{
}
