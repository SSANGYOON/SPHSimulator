#pragma once
class SimulationObject
{
public:
	SimulationObject();
	virtual ~SimulationObject();

	virtual void Update();
	virtual void Render();
	virtual void ImGuiRender();

private:
	string name;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Matrix srt;

public:
	inline const string& GetName() { return name; }
	inline void SetName(const string& name) { this->name = name; }
};

