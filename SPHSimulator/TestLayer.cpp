#include "pch.h"
#include "TestLayer.h"
#include "Resources.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "Mesh.h"
#include "StructuredBuffer.h"
#include <random>
#include <algorithm>

#include "Camera.h"
#include "SPHSystem.h"
#include "ConstantBuffer.h"
#include "Graphics.h"

SY::TestLayer::TestLayer()
{
}

SY::TestLayer::~TestLayer()
{
}

void SY::TestLayer::OnAttach()
{
	SPHSettings sphSettings(0.02f, 1000, 1, 1.04, 0.15f, -9.8, 0.2f);
	sphSystem = new SPHSystem(15, sphSettings);
}

void SY::TestLayer::OnDetach()
{
	delete sphSystem;
}

void SY::TestLayer::OnUpdate(float timestep)
{

	sphSystem->update(timestep);
	sphSystem->draw();
}

void SY::TestLayer::OnImGuiRender()
{
	static int numParticles = 15;
	static float nMass = 0.02;
	static float nh = 0.15f;
	static float nRest = 10.f;
	static float nVisco = 3.5f;
	static float gasConst = 1.f;
	static int counter = 0;

	ImGui::Begin("SPH debug");                          // Create GUI window

	ImGui::Text("Change values for the simulation. Press RESET to commit changes");

	ImGui::SliderInt("Number of Particles", &numParticles, 10, 600);
	ImGui::SliderFloat("Mass of Particles", &nMass, 0.001f, 1.f);
	ImGui::SliderFloat("Support Radius", &nh, 0.001f, 1.f);
	ImGui::SliderFloat("Rest Density", &nRest, 0.001f, 2000.f);
	ImGui::SliderFloat("Viscosity Constant", &nVisco, 0.001f, 5.f);
	ImGui::SliderFloat("Gas Constant", &gasConst, 0.001f, 5.f);

	if (ImGui::Button("RESET")) {
		delete sphSystem;
		SPHSettings sphSettings(nMass, nRest, gasConst, nVisco, nh, -9.8, 1.f);
		sphSystem = new SPHSystem(numParticles, sphSettings);
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void SY::TestLayer::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(TestLayer::OnWindowResize));
}

bool SY::TestLayer::OnWindowResize(WindowResizeEvent& e)
{
	sphSystem->ResizeRatio(e.GetWidth(), e.GetHeight());
	return false;
}
