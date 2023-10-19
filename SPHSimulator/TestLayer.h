#include "Layer.h"
#include "ApplicationEvent.h"
#include "Camera.h"
#include "SPHSystem.h"

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

	private:
		class SPHSystem* sphSystem;

		int prevTime, currentTime;
		float deltaTime;

	};
}