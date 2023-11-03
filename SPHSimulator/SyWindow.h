#pragma once
#include "Event.h"
#include <sstream>

namespace SY {
	struct WindowProps
	{
		std::wstring Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::wstring& title = L"SY Engine",
			uint32_t width = 1280,
			uint32_t height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	class SyWindow
	{
	public:

		SyWindow(const WindowProps& props);
		~SyWindow();

		void OnUpdate();

		uint32_t GetWidth();
		uint32_t GetHeight();

		HWND GetHwnd() { return m_Window; }

		void SetVSync(bool enabled);
		bool IsVSync() const;

		static unique_ptr<SyWindow> Create(const WindowProps& props = WindowProps());

	private:
		void Init(const WindowProps& props);
		void shutdown();

		HWND m_Window;
		WNDCLASSEXW wc;
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}