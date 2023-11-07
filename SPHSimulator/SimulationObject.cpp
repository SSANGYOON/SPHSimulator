#include "pch.h"
#include "SimulationObject.h"
#include "ImGuizmo.h"
#include "Graphics.h"

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

void SimulationObject::ManipulateGuizmo(Matrix& view, Matrix& projection)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    WindowInfo Info = GEngine->GetWindow();

    ImGuizmo::SetRect(0, 0, Info.width, Info.height);

    static int gizmoType = -1;

    if (INPUT->GetKeyState(KEY_TYPE::Q) == KEY_STATE::DOWN)
        gizmoType = 1;
    if (INPUT->GetKeyState(KEY_TYPE::T) == KEY_STATE::DOWN)
        gizmoType = ImGuizmo::OPERATION::TRANSLATE;
    if (INPUT->GetKeyState(KEY_TYPE::R) == KEY_STATE::DOWN)
        gizmoType = ImGuizmo::OPERATION::ROTATE;
    if (INPUT->GetKeyState(KEY_TYPE::S) == KEY_STATE::DOWN)
        gizmoType = ImGuizmo::OPERATION::SCALE;

    bool snap = INPUT->GetKeyState(KEY_TYPE::LBUTTON) == KEY_STATE::PRESS;
    float snapValue = 0.5f; // Snap to 0.5m for translation/scale
    // Snap to 45 degrees for rotation
    if (gizmoType == ImGuizmo::OPERATION::ROTATE)
        snapValue = 45.0f;

    float snapValues[3] = { snapValue, snapValue, snapValue };
    ImGuizmo::Manipulate(reinterpret_cast<float*>(&view), reinterpret_cast<const float*>(&projection),
        (ImGuizmo::OPERATION)gizmoType, ImGuizmo::WORLD, reinterpret_cast<float*>(&srt),
        nullptr, snap ? snapValues : nullptr);

    if (ImGuizmo::IsUsing())
    {
        Vector3 translation, scale;
        Quaternion rotation;

        srt.Decompose(scale, rotation, translation);
        position =translation;
        this->rotation = rotation;
        this->scale = scale;
    }
}
