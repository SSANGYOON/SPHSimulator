#include "Layer.h"

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

	private:
		unique_ptr<StructuredBuffer> randomNumbersBuffer;
		unique_ptr<StructuredBuffer> countedNumbersBuffer;
		unique_ptr<StructuredBuffer> prefixSumBuffer;
		unique_ptr<StructuredBuffer> groupSumBuffer;
		unique_ptr<StructuredBuffer> sortedResultBuffer;
		UINT32 sortedResult[2048];
		UINT32 gpuSortedResult[2048];
	};
}