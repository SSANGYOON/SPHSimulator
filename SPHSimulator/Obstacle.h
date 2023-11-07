#pragma once
#include "SimulationObject.h"
#include "Mesh.h"
class Obstacle : public SimulationObject
{
public:
	Obstacle();
	virtual ~Obstacle();

	virtual void Render(class Camera* Cam) override;
	

private:
	shared_ptr<Mesh> obstacleMesh;

public:
	inline void SetMesh(shared_ptr<Mesh> mesh) { obstacleMesh = mesh; }
};

