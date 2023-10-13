#pragma once

#include <mutex>
#include "LayerStack.h"
#include "SyWindow.h"

int main(int argc, char** argv);

namespace SY {

	class Layer;
	class ImGuiLayer;
	class SyWindow;
	class WindowCloseEvent;
	class WindowResizeEvent;
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			assert(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::wstring Name = L"Hazel Application";
		std::wstring WorkingDirectory = L"";
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		SyWindow& GetWindow() { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		void SetMinimized(bool minimized) { m_Minimized = minimized; }
		static Application& Get() { return *s_Instance; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		void SubmitToMainThread(const std::function<void()>& function);
	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification m_Specification;
		unique_ptr<SyWindow> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);

		static Application* CreateApplication(const wstring& name, ApplicationCommandLineArgs args)
		{
			ApplicationSpecification spec;
			spec.Name = name;
			spec.CommandLineArgs = args;

			return new Application(spec);
		}
	};

}