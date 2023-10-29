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
	void updateParticles(float deltaTime);

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

	unique_ptr<InstancingBuffer> Intances;
	unique_ptr<class IndirectBuffer> ParticleIndirect;

	unique_ptr<StructuredBuffer> particleBuffer;
	unique_ptr<StructuredBuffer> hashToParticleIndexTable;
	
	unique_ptr<StructuredBuffer> ParticleWorldMatrixes;

	//Particle GPUSortedParticle[32768];
	//UINT Table[32768];

	float WinX = 4;
	float WinY = 3;

	UINT GetHashFromCell(int x, int y, int z);
	UINT GetHashOnCPU(Particle& p);
};