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

	unique_ptr<Texture> backgroundTexture;
	unique_ptr<Texture> normalMap;
	unique_ptr<Texture> obstacleDepth;

	vector<shared_ptr<class SimulationObject>> simulationObjects;
	
	float blurDepthFalloff = 24.f;
	int filterRadius = 30;
	Vector3 SpecularColor = Vector3::One;
	float SpecularIntensity = 1.f;
	float SpecularPower = 100.f;
	float absorbanceCoff = 0.1f;
	Vector3 FluidColor = Vector3(0.0, 0.5, 0.9);

	Vector3 boundaryCentor = Vector3(0, 0, 0);
	Vector3 boundarySize = Vector3(10, 10, 10);

	Matrix View;
	Matrix Projection;

	//Particle GPUSortedParticle[32768];
	//UINT Table[32768];

	float WinX = 4;
	float WinY = 3;

	void DrawBoundary(Camera* Cam);
	void DrawWireFrame(const Vector3& size, const Quaternion& rotation, const Vector3& translation, Camera* Cam);
};