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
	float Azimuth; // Rotation of the camera eye position around Y axis (radian)
	float Incline; // Angle of the camear eye postion over the XZ plane (radian)

	Matrix ViewProjectionMatrix;

public:
	inline void SetAspect(float a) { Aspect = a; }
	inline void SetDistance(float d) { Distance = d; }
	inline void SetAzimuth(float a) { Azimuth = a; }
	inline void SetIncline(float i) { Incline = i; }

	inline float GetDistance() { return Distance; };
	inline float GetAzimuth() { return Azimuth; }
	inline float GetIncline() { return Incline; }

	const Matrix& GetViewProjectionMatrix() { return ViewProjectionMatrix; }
};

