#include "pch.h"
#include "SimulationObject.h"


SimulationObject::SimulationObject()
{
}

SimulationObject::~SimulationObject()
{
}

void SimulationObject::Update()
{
	srt = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
}

void SimulationObject::Render()
{
}

void SimulationObject::ImGuiRender()
{
	char buf[255];
	memcpy_s(buf, 255, name.c_str(), name.length() + 1);
	ImGui::InputText("name", buf, 255);

	name = string(buf);

	ImGui::DragFloat3("position", reinterpret_cast<float*>(&position));

	Vector3 eulerAngle = rotation.ToEuler() / XM_PI * 180.f;

	ImGui::DragFloat3("rotation", reinterpret_cast<float*>(&eulerAngle));

	rotation = Quaternion::CreateFromYawPitchRoll(eulerAngle / 180.f * XM_PI);

	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&scale));
}
