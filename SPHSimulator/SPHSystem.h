#pragma once

struct SPHSettings
{
	SPHSettings(
		float mass, float restDensity, float gasConst, float viscosity,
		float h, float g, float tension);

	Matrix sphereScale;
	float poly6, spikyGrad, spikyLap, gasConstant, mass, h2, selfDens,
		restDensity, viscosity, h, g, tension, massPoly6Product;
};

class SPHSystem
{
public:
	SPHSystem(UINT32 particleCubeWidth, const SPHSettings& settings);
	~SPHSystem();

	struct Particle* particles;
	UINT32 particleCount;

	void update(float deltaTime);

	void draw();

	void reset();
	void startSimulation();

private:
	SPHSettings settings;
	size_t particleCubeWidth;

	bool started;
	void InitParticles();
	shared_ptr<class Mesh> sphere;
	Matrix* sphereModelMtxs;
};