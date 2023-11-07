#pragma once
class SimulationObject
{
public:
	SimulationObject();
	virtual ~SimulationObject();

	virtual void Update();
	virtual void Render(class Camera* Cam) abstract;
	virtual void ImGuiRender();
	void ManipulateGuizmo(Matrix& view, Matrix& projection);

	const Matrix& GetTransform() { return srt; };

protected:
	string name;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale = Vector3::One;
	Matrix srt;

public:
	inline const string& GetName() { return name; }
	inline void SetName(const string& name) { this->name = name; }

	void SetPosition(const Vector3& position) { this->position = position; }
	void SetRotation(const Quaternion& rotation) { this->rotation = rotation; }
	void SetScale(const Vector3& scale) { this->scale = scale; }
};

