
#include "pch.h"
#include "SPHSimulator.h"
#include "Application.h"

int main(int argc, char** argv)
{
	auto app = SY::Application::CreateApplication(L"Sphsimulator", { argc, argv });

	app->Run();

	delete app;
}
