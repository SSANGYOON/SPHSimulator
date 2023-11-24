#pragma once

class Timer
{
	DECLARE_SINGLE(Timer);

public:
	float DeltaTime() { return mDeltaTime * mTimeScale; }

	void Init();
	float GetTimeScale() { return mTimeScale; }
	float GetCPUFreq() { return static_cast<float>(mCpuFrequency.QuadPart); }
	void SetTimeScale(float timeScale) { mTimeScale = timeScale; }
	void Update();

private:
	LARGE_INTEGER	mCpuFrequency = {};
	LARGE_INTEGER   mPrevFrequency = {};
	LARGE_INTEGER	mCurFrequency = {};

	float			mDeltaTime = 0.f;
	float			mOneSecond = 0.f;
	float			mTimeScale = 1.f;
};