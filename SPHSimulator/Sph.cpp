#include "pch.h"
#include "Sph.h"

void updateParticles(Particle* particles, Matrix* particleTransforms, const UINT32 particleCount, const SPHSettings& settings, float deltaTime)
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

	Cam = make_unique<Camera>();

	SPHSettings sphSettings(0.02f, 1000, 1, 1.04, 0.15f, -9.8f, 0.2f);
	sphSystem = new SPHSystem(15, sphSettings);
}
