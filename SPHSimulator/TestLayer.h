#include "Layer.h"

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
	};
}