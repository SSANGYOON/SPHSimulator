#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	Reset();
}

void Camera::Update()
{
	Matrix world = Matrix::Identity;
	world._32 = -Distance;
	world = Matrix::CreateRotationY(-Azimuth) * Matrix::CreateRotationX(-Incline) * world;

	Matrix view = world.Invert();
	Matrix project = Matrix::CreatePerspectiveFieldOfViewLH(FOV, Aspect, NearClip, FarClip);


	ViewProjectionMatrix = project * view;
}

void Camera::Reset()
{
	FOV = DirectX::XM_PIDIV4;
	Aspect = 1.33f;
	NearClip = 0.1f;
	FarClip = 100.f;

	Distance = 10.0f;
	Azimuth = 0.0f;
	Incline = DirectX::XM_PIDIV2 / 9.0f;
}