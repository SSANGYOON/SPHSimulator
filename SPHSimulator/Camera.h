#pragma once
class Camera
{
public:
	Camera();
	
	void Update();
	void Reset();

private:
	float FOV ; //radian
	float Aspect;
	float NearClip;
	float FarClip;

	float Distance; // Distance of the camera eye position to the origin(meters)
	Quaternion rot;

	Matrix ViewMatrix;
	Matrix ProjectionMatrix;

public:
	inline void SetAspect(float a) { Aspect = a; }
	inline void SetDistance(float d) { Distance = -d; }
	inline void SetRotation(Quaternion q) { rot = q; }
	inline void ApplayRotation(Quaternion q) { rot = q * rot; }

	inline float GetDistance() { return -Distance; };
	inline Quaternion GetRoation() { return rot; }
	inline float GetFarClip() const { return FarClip; }
	inline float GetNearClip() const { return NearClip; }

	const Matrix& GetViewMatrix() { return ViewMatrix; }
	const Matrix& GetProjectionMatrix() { return ProjectionMatrix; }
};

