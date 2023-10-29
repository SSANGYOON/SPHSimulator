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
	world = world * Matrix::CreateRotationY(-Azimuth) * Matrix::CreateRotationX(-Incline) ;

	Vector4 t = { 0, 0, 0, 1 };
	
	Matrix view = world.Invert();

	Vector4 v = Vector4::Transform(t, view);
	Matrix project = ::XMMatrixPerspectiveFovLH(FOV, Aspect, NearClip, FarClip);


	ViewProjectionMatrix = view * project;
	Vector4 r = Vector4::Transform(t, ViewProjectionMatrix);
}

void Camera::Reset()
{
	FOV = DirectX::XM_PIDIV4;
	Aspect = 1.33f;
	NearClip = 0.1f;
	FarClip = 100.f;

	Distance = -10;
	Azimuth = 0;
	Incline = 0;
}
