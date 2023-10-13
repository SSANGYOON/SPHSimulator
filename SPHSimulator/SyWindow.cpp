#include "pch.h"
#include "SyWindow.h"
#include "ApplicationEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "Application.h"
#include "Graphics.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) {

			SY::WindowResizeEvent e((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			SY::Application::Get().OnEvent(e);
			return true;
		}
		else
		{
			SY::Application::Get().SetMinimized(true);
			return true;
		}
	case WM_CLOSE:
	{
		SY::WindowCloseEvent e;
		SY::Application::Get().OnEvent(e);
		return true;
	}
	case WM_KEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		KEY_TYPE type = (KEY_TYPE)wParam;

		if (msg == WM_LBUTTONDOWN)
			type = KEY_TYPE::LBUTTON;
		else if (msg == WM_RBUTTONDOWN)
			type = KEY_TYPE::RBUTTON;
		else if (msg == WM_MBUTTONDOWN)
			type = KEY_TYPE::MBUTTON;
		SY::KeyPressedEvent e(type, (lParam & 1 << 30) > 0);
		SY::Application::Get().OnEvent(e);

		return true;
	}
	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		KEY_TYPE type = (KEY_TYPE)wParam;

		if (msg == WM_LBUTTONUP)
			type = KEY_TYPE::LBUTTON;
		else if (msg == WM_RBUTTONUP)
			type = KEY_TYPE::RBUTTON;
		else if (msg == WM_MBUTTONUP)
			type = KEY_TYPE::MBUTTON;

		SY::KeyReleasedEvent e(type);
		SY::Application::Get().OnEvent(e);
		return true;
	}

	case WM_MOUSEMOVE:
	{
		SY::MouseMovedEvent e((float)LOWORD(lParam), (float)HIWORD(lParam));
		SY::Application::Get().OnEvent(e);
		return true;
	}
	case WM_MOUSEWHEEL:
	{
		SY::MouseScrolledEvent e(0.f, GET_WHEEL_DELTA_WPARAM(wParam));
		SY::Application::Get().OnEvent(e);
		return true;
	}
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

namespace SY {

	SyWindow::SyWindow(const WindowProps& props)
	{
		Init(props);
	}
	SyWindow::~SyWindow()
	{
		shutdown();
	}
	void SyWindow::OnUpdate()
	{
		MSG msg;
		SY::Application::Get().SetMinimized(false);
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				return;
		}
	}
	uint32_t SyWindow::GetWidth()
	{
		return m_Data.Width;
	}
	uint32_t SyWindow::GetHeight()
	{
		return m_Data.Height;
	}
	void SyWindow::SetVSync(bool enabled)
	{
		m_Data.VSync = enabled;
		GEngine->SetVSync(enabled);
	}
	bool SyWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
	unique_ptr<SyWindow> SyWindow::Create(const WindowProps& props)
	{
		auto window = make_unique<SyWindow>(props);
		return window;
	}

	void SyWindow::Init(const WindowProps& props)
	{
		wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"HAZEL", nullptr };
		::RegisterClassExW(&wc);
		m_Window = ::CreateWindowW(wc.lpszClassName, props.Title.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, props.Width, props.Height, nullptr, nullptr, wc.hInstance, nullptr);

		if (FAILED(GEngine->Init({ m_Window, props.Width, props.Height, true })))
		{
			::UnregisterClassW(wc.lpszClassName, wc.hInstance);
			assert(false);
		}

		::ShowWindow(m_Window, SW_SHOWDEFAULT);
		::UpdateWindow(m_Window);
	}
	void SyWindow::shutdown()
	{
		::DestroyWindow(m_Window);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
	}
}