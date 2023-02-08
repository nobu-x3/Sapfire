#include "engpch.h"
#include "Sapfire/core/Application.h"

int main(int argc, char* argv[])
{
	Sapfire::Application* application = Sapfire::CreateApplication();
	application->Run();
	delete application;
	return 0;
}
