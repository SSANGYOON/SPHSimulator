#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	Reset();
}

void Camera::Update()
{
	Matrix world = Matrix::Identity;
	world._43 = Distance;
	world = world * Matrix::CreateFromQuaternion(rot) ;
	
	ViewMatrix = world.Invert();
	ProjectionMatrix = ::XMMatrixPerspectiveFovLH(FOV, Aspect, NearClip, FarClip);

	Vector4 v = Vector4(Aspect * FarClip / 2, FarClip / 2, FarClip / 2, 1);

	Vector4 r = Vector4::Transform(v, ProjectionMatrix);
}

void Camera::Reset()
{
	FOV = DirectX::XM_PIDIV4;
	Aspect = 1.33f;
	NearClip = 0.1f;
	FarClip = 100.f;

	Distance = -10;
	rot = Quaternion::Identity;
}
