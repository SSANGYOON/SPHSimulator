#pragma once
class SimulationObject
{
public:
	SimulationObject();
	virtual ~SimulationObject();

	virtual void Update();
	virtual void Render(class Camera* Cam) abstract;
	virtual void ImGuiRender();

protected:
	string name;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale = Vector3::One;
	Matrix srt;

public:
	inline const string& GetName() { return name; }
	inline void SetName(const string& name) { this->name = name; }
};

