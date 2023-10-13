#include "pch.h"
#include "Timer.h"

void Timer::Init()
{
    //CPU �� �ʴ� �ݺ��Ǵ� ���ļ��� ���´�.
    QueryPerformanceFrequency(&mCpuFrequency);

    //���α׷��� ������������ CPU Ŭ�� ��
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