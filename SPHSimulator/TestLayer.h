#include "Layer.h"
#include "ApplicationEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "SPHSystem.h"
#include "Camera.h"

class StructuredBuffer;
namespace SY {

	class TestLayer : public Layer
	{
	public:
		TestLayer();
		virtual ~TestLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(float timestep) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnKeyEvent(KeyPressedEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);

	private:
		class SPHSystem* sphSystem;

		int prevTime, currentTime;
		float deltaTime;

		unique_ptr<class Camera> Cam;

		float WinX, WinY;

		int MouseX, MouseY;

	};
}