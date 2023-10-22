#pragma once
#include "particle.h"

struct SPHSettings
{
	SPHSettings(
		float mass, float restDensity, float gasConst, float viscosity,
		float h, float g, float tension);

	Matrix sphereScale;
	float poly6, spikyGrad, spikyLap, gasConstant, mass, h2, selfDens,
		restDensity, viscosity, h, g, tension, massPoly6Product;
};

class StructuredBuffer;
class InstancingBuffer;
class SPHSystem
{
public:
	SPHSystem(UINT32 particleCubeWidth, const SPHSettings& settings);
	~SPHSystem();

	struct Particle* particles;
	UINT32 particleCount;

	void update(float deltaTime);
	void updateParticles(Matrix* sphereModelMtxs, float deltaTime);

	void draw(class Camera* Cam);

	void reset();
	void startSimulation();
	UINT MaxParticle = 1 << 18;

private:
	SPHSettings settings;
	size_t particleCubeWidth;
	

	bool started;
	void InitParticles();
	shared_ptr<class Mesh> sphere;
	Matrix* sphereModelMtxs;
	unique_ptr<InstancingBuffer> Intances;

	unique_ptr<StructuredBuffer> particleBuffer;
	//unique_ptr<StructuredBuffer> hashcountedBuffer;
	//unique_ptr<StructuredBuffer> offsetBuffer;
	//unique_ptr<StructuredBuffer> prefixSumBuffer;
	//unique_ptr<StructuredBuffer> groupSumBuffer;
	//unique_ptr<StructuredBuffer> sortedResultBuffer;
	unique_ptr<StructuredBuffer> hashToParticleIndexTable;

	Particle GPUSortedParticle[4096];
	UINT NeighborTable[4096];

	float WinX = 4;
	float WinY = 3;

	UINT GetHashFromCell(int x, int y, int z);
	UINT GetHashOnCPU(Particle& p);
};