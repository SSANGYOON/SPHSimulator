#pragma once

#include "framework.h"

#define WIN32_LEAN_AND_MEAN
#define _HAS_STD_BYTE 0
#define NOMINMAX

#include <windows.h>
#include <tchar.h>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <bitset>
#include <iostream>
#include <fstream>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

#include <wrl.h>
#include <DirectXMath.h>
#include "SimpleMath.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment(lib, "Winmm.lib")

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

using namespace DirectX::SimpleMath;

#define DECLARE_SINGLE(type)		\
private:							\
	type() {}						\
	~type() {}						\
public:								\
	static type* GetInstance()		\
	{								\
		static type instance;		\
		return &instance;			\
	}								\

#define GET_SINGLE(type)	type::GetInstance()

#include "imgui.h"

#include "Input.h"
#include "Timer.h"

#define INPUT GET_SINGLE(Input)
#define TIMER GET_SINGLE(Timer)

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

extern unique_ptr<class Graphics> GEngine;

#define DEVICE				GEngine->GetDevice()
#define CONTEXT				GEngine->GetContext()
#define SWAPCHAIN			GEngine->GetSwapChain()

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		// 디버깅할 때 여기에 breakpoint 설정
		throw std::exception();
	}
}

UINT NextPowerOf2(UINT number);