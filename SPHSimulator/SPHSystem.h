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
class Texture;
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
	void ImGUIRender();

	void reset();
	void startSimulation();
	UINT MaxParticle = 1 << 18;

private:
	SPHSettings settings;
	size_t particleCubeWidth;
	

	bool started;
	void InitParticles();
	shared_ptr<class Mesh> sphere;
	shared_ptr<Texture> cubeMap;

	unique_ptr<InstancingBuffer> Intances;
	unique_ptr<StructuredBuffer> IndirectGPU;
	unique_ptr<class IndirectBuffer> ParticleIndirect;

	unique_ptr<StructuredBuffer> particleBuffer;
	unique_ptr<StructuredBuffer> hashToParticleIndexTable;
	
	unique_ptr<StructuredBuffer> ParticleWorldMatrixes;

	unique_ptr<Texture> SceneFrontDepth;
	unique_ptr<Texture> SceneBackwardDepth;

	unique_ptr<Texture> horizontalBlurredFrontDepth;
	unique_ptr<Texture> horizontalBlurredBackwardDepth;

	unique_ptr<Texture> thicknessMap;
	unique_ptr<Texture> normalMap;
	
	float blurDepthFalloff = 56.f;
	int filterRadius = 10;
	Vector3 SpecularColor = Vector3::One;
	float SpecularIntensity = 1.f;
	float SpecularPower = 100.f;
	float absorbanceCoff = 0.1f;
	//Particle GPUSortedParticle[32768];
	//UINT Table[32768];

	float WinX = 4;
	float WinY = 3;

	UINT GetHashFromCell(int x, int y, int z);
	UINT GetHashOnCPU(Particle& p);
};