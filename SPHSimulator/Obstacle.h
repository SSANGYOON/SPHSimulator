#pragma once
#include "SimulationObject.h"
#include "VoxelGrid.h"

class StructuredBuffer;
class Obstacle : public SimulationObject
{
public:
	Obstacle();
	virtual ~Obstacle();

	virtual void Render(class Camera* Cam) override;
	void ComputeVolumeMap(float h);

	void BindObstacleBuffer();
	void ClearObstacleBuffer();

private:
	shared_ptr<class Mesh> obstacleMesh;
	unique_ptr<StructuredBuffer> sdfBuffer;
	unique_ptr<StructuredBuffer> volumeBuffer;
	VoxelGrid<float> sdf;

public:
	inline void SetMesh(shared_ptr<class Mesh> mesh) { obstacleMesh = mesh; }
};

