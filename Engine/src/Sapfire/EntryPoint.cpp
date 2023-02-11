#include "engpch.h"
#include "Sapfire/core/Application.h"
#include "Sapfire/tools/Profiling.h"

int main(int argc, char* argv[])
{
	PROFILE_BEGIN_SESSION("Startup", "SapfireProfile_Startup.json");
	Sapfire::Application* application = Sapfire::CreateApplication();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Runtime", "SapfireProfile_Runtime.json");
	application->Run();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Shutdown", "SapfireProfile_Shutdown.json");
	delete application;
	PROFILE_END_SESSION();

	return 0;
}
