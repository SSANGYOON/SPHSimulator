#include "pch.h"
#include "TestLayer.h"
#include "Resources.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "Mesh.h"
#include "StructuredBuffer.h"
#include <random>
#include <algorithm>

#include "SPHSystem.h"
#include "ConstantBuffer.h"
#include "Graphics.h"
#include <algorithm>

SY::TestLayer::TestLayer()
{
}

SY::TestLayer::~TestLayer()
{
}

void SY::TestLayer::OnAttach()
{
	SPHSettings sphSettings(3, 1, 1, 3.5, 0.15, -9.8, 0.2f);
	sphSystem = new SPHSystem(15, sphSettings);
	Cam = make_unique<Camera>();
}

void SY::TestLayer::OnDetach()
{
	delete sphSystem;
}

void SY::TestLayer::OnUpdate(float timestep)
{
	Cam->Update();
	sphSystem->update(timestep);
	sphSystem->draw(Cam.get());
}

void SY::TestLayer::OnImGuiRender()
{
	static int numParticles = 15;
	static float nMass = 0.02;
	static float nh = 0.15f;
	static float nRest = 1000;
	static float nVisco = 3.5f;
	static float gasConst = 1.f;
	static int counter = 0;

	ImGui::Begin("SPH debug");                          // Create GUI window

	ImGui::Text("Change values for the simulation. Press RESET to commit changes");

	ImGui::DragInt("Number of Particles", &numParticles, 10, 600);
	ImGui::DragFloat("Mass of Particles", &nMass, 0.001f, 1.f);
	ImGui::DragFloat("Support Radius", &nh, 0.001f, 1.f);
	ImGui::DragFloat("Rest Density", &nRest, 0.001f, 2000.f);
	ImGui::DragFloat("Viscosity Constant", &nVisco, 0.001f, 5.f);
	ImGui::DragFloat("Gas Constant", &gasConst, 0.001f, 5.f);

	if (ImGui::Button("RESET")) {
		delete sphSystem;
		SPHSettings sphSettings(nMass, nRest, gasConst, nVisco, nh, -9.8, 1.f);
		sphSystem = new SPHSystem(numParticles, sphSettings);
		Cam->SetAspect(WinX / WinY);
		Cam->SetAzimuth(0);
		Cam->SetIncline(0);
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void SY::TestLayer::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(TestLayer::OnWindowResize));
	dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(TestLayer::OnKeyEvent));
	dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(TestLayer::OnMouseMoved));
}

bool SY::TestLayer::OnWindowResize(WindowResizeEvent& e)
{
	WinX = e.GetWidth();
	WinY = e.GetHeight();
	Cam->SetAspect(WinX / WinY);
	return false;
}

bool SY::TestLayer::OnKeyEvent(KeyPressedEvent& e)
{
	switch (e.GetKeyCode()) {
	case KEY_TYPE::C:		// Escape
		sphSystem->startSimulation();
		break;
	case KEY_TYPE::R:
		Cam->Reset();
		Cam->SetAspect(float(WinX) / float(WinY));
		sphSystem->reset();
		break;
	case KEY_TYPE::ESCAPE:
		GEngine->Quit();
		break;
	}
	return false;
}

bool SY::TestLayer::OnMouseMoved(MouseMovedEvent& e)
{
	int nx = (int)e.GetX();
	int ny = (int)e.GetY();

	int maxDelta = 100;


	int dx = std::clamp(nx - MouseX, -maxDelta, maxDelta);
	int dy = std::clamp(-(ny - MouseY), -maxDelta, maxDelta);

	MouseX = nx;
	MouseY = ny;

	bool LeftDown = INPUT->GetKeyState(KEY_TYPE::LBUTTON) == KEY_STATE::PRESS;
	bool RightDown = INPUT->GetKeyState(KEY_TYPE::RBUTTON) == KEY_STATE::PRESS;

	// Move camera
	// NOTE: this should really be part of Camera::Update()
	if (LeftDown) {
		const float rate = XM_PIDIV2 / 90;
		Cam->SetAzimuth(Cam->GetAzimuth() + dx * rate);
		Cam->SetIncline(clamp(Cam->GetIncline() - dy * rate, -XM_PIDIV2, XM_PIDIV2));
	}
	if (RightDown) {
		const float rate = 0.005f;
		float dist = clamp(Cam->GetDistance() * (1.0f - dx * rate), 0.01f, 1000.0f);
		Cam->SetDistance(dist);
	}

	return true;
}
