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
	/*randomNumbersBuffer = make_unique<StructuredBuffer>();
	UINT32* randomNumbers = new UINT[2048];

	countedNumbersBuffer = make_unique<StructuredBuffer>();

	std::random_device rd; // Seed the random number generator
	std::mt19937 gen(rd()); // Mersenne Twister pseudo-random generator

	int lower_bound = 1; // Your lower bound (inclusive)
	int upper_bound = 2047; // Your upper bound (inclusive)

	std::uniform_int_distribution<int> distribution(lower_bound, upper_bound);

	for (int i = 0; i < 2048; i++)
	{
		randomNumbers[i] = distribution(gen); // Generate a random number in the specified range
	}

	sort(randomNumbers, randomNumbers + 2048);

	bool r = randomNumbersBuffer->Create(sizeof(UINT32), 2048, randomNumbers, true, false);


	for (int i = 0; i < 2048; i++)
	{
		sortedResult[i] = randomNumbers[i];
	}

	delete[] randomNumbers;
	countedNumbersBuffer->Create(sizeof(UINT32), 2048, nullptr, true, false);
	prefixSumBuffer = make_unique<StructuredBuffer>();
	prefixSumBuffer->Create(sizeof(UINT32), 2048, nullptr, true, false);
	groupSumBuffer = make_unique<StructuredBuffer>();
	groupSumBuffer->Create(sizeof(UINT32), 4, nullptr, true, false);
	sortedResultBuffer = make_unique<StructuredBuffer>();
	sortedResultBuffer->Create(sizeof(UINT32), 2048, nullptr, true, true);*/

	

	SPHSettings sphSettings(0.02f, 1000, 1, 1.04, 0.15f, -9.8f, 0.2f);
	sphSystem = new SPHSystem(15, sphSettings);
}

void SY::TestLayer::OnDetach()
{
	delete sphSystem;
}

void SY::TestLayer::OnUpdate(float timestep)
{

	//sphSystem->update(deltaTime);
	sphSystem->draw();
}

void SY::TestLayer::OnImGuiRender()
{
	static int numParticles = 15;
	static float nMass = 0.02;
	static float nh = 0.15f;
	static float nRest = 1000.f;
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
