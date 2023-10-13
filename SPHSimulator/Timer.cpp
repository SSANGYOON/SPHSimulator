#include "pch.h"
#include "Timer.h"

void Timer::Init()
{
    //CPU 의 초당 반복되는 주파수를 얻어온다.
    QueryPerformanceFrequency(&mCpuFrequency);

    //프로그램을 시작했을때의 CPU 클럭 수
    QueryPerformanceCounter(&mPrevFrequency);
}

void Timer::Update()
{
    QueryPerformanceCounter(&mCurFrequency);

    float differenceInFrequancy
        = static_cast<float>((mCurFrequency.QuadPart - mPrevFrequency.QuadPart));

    mDeltaTime = differenceInFrequancy / static_cast<float>(mCpuFrequency.QuadPart);
    mPrevFrequency.QuadPart = mCurFrequency.QuadPart;
}