// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.
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