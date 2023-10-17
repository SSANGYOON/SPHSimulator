#include "pch.h"
#include "TestLayer.h"
#include "Resources.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "Mesh.h"
#include "StructuredBuffer.h"
#include <random>
#include <algorithm>

SY::TestLayer::TestLayer()
{
}

SY::TestLayer::~TestLayer()
{
}

void SY::TestLayer::OnAttach()
{
	randomNumbersBuffer = make_unique<StructuredBuffer>();
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
	sortedResultBuffer->Create(sizeof(UINT32), 2048, nullptr, true, true);
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

	auto CountShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CountingShader");
	CountShader->SetThreadGroups(2, 1, 1);
	randomNumbersBuffer->BindUAV(0);
	countedNumbersBuffer->BindUAV(3);
	CountShader->Dispatch();

	auto prefixSumOnGroupShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"PrefixSumOnThreadGroupShader");
	prefixSumOnGroupShader->SetThreadGroups(2, 1, 1);
	prefixSumBuffer->BindUAV(1);
	groupSumBuffer->BindUAV(2);
	prefixSumOnGroupShader->Dispatch();

	auto prefixSumOnGroupSumShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"PrefixSumOnGroupSumShader");
	prefixSumOnGroupSumShader->SetThreadGroups(1, 1, 1);
	prefixSumOnGroupSumShader->Dispatch();

	auto prefixCompleteShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"PrefixSumCompleteShader");
	prefixCompleteShader->SetThreadGroups(2, 1, 1);
	prefixCompleteShader->Dispatch();

	auto countingSortCompleteShader = GET_SINGLE(Resources)->Find<ComputeShader>(L"CompleteCountingSort");
	countingSortCompleteShader->SetThreadGroups(2, 1, 1);
	sortedResultBuffer->BindUAV(4);
	countingSortCompleteShader->Dispatch();

	sortedResultBuffer->GetData(gpuSortedResult);
	int a = 0;
}

void SY::TestLayer::OnImGuiRender()
{
}

void SY::TestLayer::OnEvent(Event& e)
{
}
