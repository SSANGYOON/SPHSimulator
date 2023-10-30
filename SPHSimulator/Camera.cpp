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
	
	ViewMatrix = world.Invert();
	ProjectionMatrix = ::XMMatrixPerspectiveFovLH(FOV, Aspect, NearClip, FarClip);
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
